#define _GNU_SOURCE

#include <sched.h>
#include <stdlib.h>
#include <float.h>
#include <stdint.h>
//#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <limits.h>
#include "rts_taskset.h"

#define PERIOD_MAX_US INT_MAX // as defined in /proc/sys/kernel/sched_rt_period_us
#define PERIOD_MIN_US 1

//------------------------------------------------------------------------------
// HYPERBOLIC BOUND: perform the sched. analysis under fp
//------------------------------------------------------------------------------

/**
 * @brief Hyperbolic bound analysis for the taskset
 */
unsigned int hyperbolic_bound(struct rts_taskset* ts) 
{
    float res;
    iterator_t iterator;
    struct rts_task* t;

    res = 1;
    iterator = rts_taskset_iterator_init(ts);
    
    for(; iterator != NULL; iterator = iterator_get_next(iterator)) 
    {
        t = rts_taskset_iterator_get_elem(iterator);

        if (t->params.period == 0 || t->params.runtime == 0)
            continue;

        res *= (t->params.runtime / (float)t->params.period) + 1;
    }

    if(res > 2)
        return 0;

    return 1;
}

// -----------------------------------------------------------------------------
// UTILITY INTERNAL METHODS
// -----------------------------------------------------------------------------

/**
 * @brief Given min/max plugin prio, normalize @p prio in that window
 */
uint32_t prio_remap(uint32_t max_prio_in, uint32_t min_prio_in, 
                    uint32_t max_prio_out, uint32_t min_prio_out, uint32_t prio_in) 
{
    float slope;
    
    if (max_prio_in - min_prio_in == 0)
        slope = 1;
    else
        slope = (max_prio_out - min_prio_out) / (float)(max_prio_in - min_prio_in);
    
    return min_prio_out + (slope * (float)(prio_in - min_prio_in));
}

/**
 * @brief Given pointer to plugin struct @p this, retrieve least loaded cpu
 */
static uint32_t least_loaded_cpu(struct rts_plugin* this)
{
    int cpu_num;
    float free_rm_max;
    int free_rm_max_cpu;

    free_rm_max_cpu = this->cpulist[0];
    free_rm_max = this->util_free_percpu[free_rm_max_cpu];

    for(int i = 1; i < this->cputot; i++)
    {
        cpu_num = this->cpulist[i];

        if (this->util_free_percpu[cpu_num] > free_rm_max)
            free_rm_max_cpu = cpu_num;
    }

    return free_rm_max_cpu;
}

static float eval_util_missing(struct rts_plugin* this, float task_util)
{
    int cpu_min;

    for(int i = 0; i < this->cputot; i++)
        if(task_util <= this->util_free_percpu[this->cpulist[i]])
            return 0;

    cpu_min = least_loaded_cpu(this);

    return task_util - this->util_free_percpu[cpu_min];
}

static uint8_t has_another_preference(struct rts_plugin* this, struct rts_task* t)
{
    char* preferred = rts_task_get_preferred_plugin(t);

    if (preferred != NULL && strcmp(this->name, preferred) != 0)
        return 1;

    return 0;
}

static int utilization_test(struct rts_plugin* this, float task_util)
{
    float missing_util = eval_util_missing(this, task_util);

    if (missing_util == 0)
        return RTS_OK;
    else
        return RTS_NO;
}

static int count_unique_periods(struct rts_plugin* this, struct rts_taskset* ts, unsigned free_cpu)
{
    struct rts_task* t_rm;
    iterator_t iterator;
    unsigned int dist_prio;
    unsigned int prec_period;

    prec_period = -1;
    dist_prio = 0;    
    iterator = rts_taskset_iterator_init(ts);
        
    for (; iterator != NULL; iterator = iterator_get_next(iterator)) 
    {
        t_rm = rts_taskset_iterator_get_elem(iterator);

        if (t_rm->cpu != free_cpu || t_rm->pluginid != this->id)
            continue;

        if (t_rm->params.period != prec_period)
            dist_prio++;

        prec_period = t_rm->params.period;
    }

    return dist_prio;
}

static void assign_priorities(struct rts_plugin* this, struct rts_taskset* ts)
{
    unsigned int free_cpu;
    unsigned int dist_prio;
    unsigned int curr_prio;
    unsigned int prec_period;
    iterator_t iterator;
    struct rts_task* t_rm;

    free_cpu = least_loaded_cpu(this);
    dist_prio = count_unique_periods(this, ts, free_cpu);

    curr_prio = 0;
    prec_period = -1;

    iterator = rts_taskset_iterator_init(ts);

    for (; iterator != NULL; iterator = iterator_get_next(iterator)) 
    {
        t_rm = rts_taskset_iterator_get_elem(iterator);

        if (t_rm->cpu != free_cpu)
            continue;

        if (t_rm->params.deadline != prec_period || t_rm->pluginid != this->id)
            curr_prio++;

        prec_period = rts_task_get_period(t_rm);
        t_rm->schedprio = prio_remap(dist_prio, 1, this->prio_max, this->prio_min, curr_prio);
    }
}

// -----------------------------------------------------------------------------
// SKELETON PLUGIN METHODS
// -----------------------------------------------------------------------------

/**
 * @brief Used by plugin to initializes itself
 */
int rts_plg_task_init(struct rts_plugin* this) 
{
    return RTS_OK;
}

/**
 * @brief Used by plugin to perform a new task admission test
 */
int rts_plg_task_accept(struct rts_plugin* this, struct rts_taskset* ts, struct rts_task* t) 
{
    float task_util;
    int test_res;

    task_util = rts_task_get_util(t);

    // if task did not specified period will be rejected
    if (rts_task_get_period(t) == 0)
        return RTS_NO;

    if (rts_task_get_ignore_admission(t))
        test_res = RTS_OK;
    else if (utilization_test(this, task_util) == RTS_OK)
        test_res = RTS_OK;
    else
        test_res = RTS_NO;

    // if not preferred plugin support is partial
    if (has_another_preference(this, t) && test_res == RTS_OK)
        test_res = RTS_PARTIAL;

    return test_res;
}

/**
 * @brief Used by plugin to perform a new admission test when task modifies parameters
 */
int rts_plg_task_change(struct rts_plugin* this, struct rts_taskset* ts, struct rts_task* t) 
{
    int test_res;

    // simulate test without task utilization in
    if (t->pluginid == this->id && t->acceptedu != 0)
        this->util_free_percpu[t->cpu] += t->acceptedu;

    test_res = rts_plg_task_accept(this, ts, t);

    // restore utilization
    if (t->pluginid == this->id && t->acceptedu != 0)
        this->util_free_percpu[t->cpu] -= t->acceptedu;

    return test_res;
}

/**
 * @brief Used by plugin to set the task as accepted
 */
void rts_plg_task_schedule(struct rts_plugin* this, struct rts_taskset* ts, struct rts_task* t)  
{
    float task_util;

    task_util = rts_task_get_util(t);

    rts_taskset_remove_top(ts);
    rts_taskset_add_sorted_pr(ts, t);

    assign_priorities(this, ts);

    t->cpu = least_loaded_cpu(this);
    t->acceptedt = rts_task_get_runtime(t);    
    t->acceptedu = task_util != -1 ? task_util : 0;
    t->pluginid = this->id;

    this->util_free_percpu[t->cpu] -= t->acceptedu;
    this->task_count_percpu[t->cpu]++;
}

/**
 * @brief Used by plugin to set rt scheduler for a task
 */
int rts_plg_task_attach(struct rts_task* t) 
{
    struct sched_param attr;
    cpu_set_t my_set;

    CPU_ZERO(&my_set);
    CPU_SET(t->cpu, &my_set);

    if(sched_setaffinity(t->tid, sizeof(cpu_set_t), &my_set) < 0)
        return RTS_ERROR;
    
    attr.sched_priority = t->schedprio;
    
    if(sched_setscheduler(t->tid, SCHED_FIFO, &attr) < 0)
        return RTS_ERROR;
   
    return RTS_OK;
}

/**
 * @brief Used by plugin to reset scheduler (other) for a task
 */
int rts_plg_task_detach(struct rts_task* t) 
{
    struct sched_param attr;
    cpu_set_t my_set;

    CPU_ZERO(&my_set);
    
    for(int i = 0; i < get_nprocs(); i++)
        CPU_SET(i, &my_set);
    
    if(sched_setaffinity(t->tid, sizeof(cpu_set_t), &my_set) < 0)
        return RTS_ERROR;
    
    attr.sched_priority = 0;
    
    if(sched_setscheduler(t->tid, SCHED_OTHER, &attr) < 0)
        return RTS_ERROR;
    
    return RTS_OK;
}

/**
 * @brief Used by plugin to perform a release of previous accepted task
 */
int rts_plg_task_release(struct rts_plugin* this, struct rts_taskset* ts, struct rts_task* t) 
{
    t->pluginid = -1;
    this->task_count_percpu[t->cpu]--;
    
    if (t->acceptedu != 0)
        this->util_free_percpu[t->cpu] += t->acceptedu;
    
    if (sched_getscheduler(t->tid) != SCHED_FIFO) // means no attached flow of ex.
        return RTS_OK;
    
    return rts_plg_task_detach(t);
}