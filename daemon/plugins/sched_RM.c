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

#define MAX_CPU 32
#define MAX_PRIO 100

static unsigned int dist_prio[MAX_CPU] = { 0 };

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
uint32_t prio_remap(uint32_t max_prio_s, uint32_t min_prio_s, uint32_t prio, unsigned cpu) 
{
    float slope;
        
    slope = (max_prio_s - min_prio_s + 1) / (float)(dist_prio[cpu]-1);
    
    return min_prio_s + slope * prio;
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

// static int count_unique_periods(struct rts_plugin* this, struct rts_taskset* ts, unsigned free_cpu)
// {
//     struct rts_task* t_rm;
//     iterator_t iterator;
//     unsigned int dist_prio;
//     unsigned int prec_period;

//     prec_period = -1;
//     dist_prio = 0;    
//     iterator = rts_taskset_iterator_init(ts);
        
//     for (; iterator != NULL; iterator = iterator_get_next(iterator)) 
//     {
//         t_rm = rts_taskset_iterator_get_elem(iterator);

//         if (t_rm->cpu != free_cpu || t_rm->pluginid != this->id)
//             continue;

//         if (t_rm->params.period != prec_period)
//             dist_prio++;

//         prec_period = t_rm->params.period;
//     }

//     return dist_prio;
// }

static void assign_priorities(struct rts_plugin* this, unsigned int cpu)
{
    unsigned int curr_prio;   // real prio
    unsigned int prec_period; // user data
    unsigned int dist_prio_idx;
    
    iterator_t iterator;
    struct rts_task* t_rm;

    curr_prio = this->prio_min-1;
    prec_period = -1;
    dist_prio_idx = 0;

    iterator = rts_taskset_iterator_init(&this->tasks[cpu]);

    if (dist_prio[cpu] > (this->prio_max - this->prio_min) + 1)
    {
        for (; iterator != NULL; iterator = iterator_get_next(iterator))
        {
            t_rm = rts_taskset_iterator_get_elem(iterator);
            
            if (prec_period == rts_task_get_period(t_rm))
            {
                rts_task_set_real_priority(t_rm, curr_prio);
            }
            else
            {
                curr_prio = prio_remap(this->prio_max, this->prio_min, dist_prio_idx++, cpu);
                rts_task_set_real_priority(t_rm, curr_prio);
                prec_period = rts_task_get_period(t_rm);
            }
        }

    }
    else
    {
        for (; iterator != NULL; iterator = iterator_get_next(iterator)) 
        {
            t_rm = rts_taskset_iterator_get_elem(iterator);

            if (prec_period == rts_task_get_period(t_rm))
            {
                rts_task_set_real_priority(t_rm, curr_prio);
            }
            else
            {
                rts_task_set_real_priority(t_rm, ++curr_prio);
                prec_period = rts_task_get_period(t_rm);
            }
        }
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
    unsigned int cpu;
    
    cpu = least_loaded_cpu(this);
    task_util = rts_task_get_util(t);

    rts_task_set_cpu(t, cpu);
    t->pluginid = this->id;

    struct node_ptr* inserted = 
        rts_taskset_add_sorted_pr(&this->tasks[cpu], t);

    if (inserted->next != NULL)
    {
        struct rts_task* next = (struct rts_task*)inserted->next->elem;

        if (rts_task_get_period(next) == t->params.period)
        {
            rts_task_set_real_priority(t, rts_task_get_real_priority(next));
        }
        else
        {
            dist_prio[cpu]++;
            assign_priorities(this, cpu);
        }
    }
    else
    {
        dist_prio[cpu]++;
        assign_priorities(this, cpu);
    }

    t->acceptedt = rts_task_get_runtime(t);    
    t->acceptedu = task_util != -1 ? task_util : 0;
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
    iterator_t iterator;
    struct rts_task* t_rm;
    
    iterator = rts_taskset_iterator_init(&this->tasks[t->cpu]);

    for (; iterator != NULL; iterator = iterator_get_next(iterator)) 
    {
        t_rm = rts_taskset_iterator_get_elem(iterator);
        if (t_rm->params.period == t->params.period 
            && t_rm->id != t->id)
        {
            dist_prio[t->cpu]--;
            break;
        }
    }

    if (t->acceptedu != 0)
        this->util_free_percpu[t->cpu] += t->acceptedu;
    
    rts_taskset_remove_by_rsvid(&this->tasks[t->cpu], t->id);

    t->pluginid = -1;
    this->task_count_percpu[t->cpu]--;

    if (sched_getscheduler(t->tid) != SCHED_FIFO) // means no attached flow of ex.
        return RTS_OK;

    return rts_plg_task_detach(t);
}