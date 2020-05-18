#define _GNU_SOURCE

#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "rts_taskset.h"
#include "rts_types.h"

// -----------------------------------------------------------------------------
// UTILITY INTERNAL METHODS
// -----------------------------------------------------------------------------

/**
 * @brief Given min/max plugin prio, normalize @p prio in that window
 */
uint32_t prio_remap(uint32_t max_prio_s, uint32_t min_prio_s, uint32_t prio) 
{
    float slope;
    int max_rr_prio;
    int min_rr_prio;

    max_rr_prio = sched_get_priority_max(SCHED_RR);
    min_rr_prio = sched_get_priority_min(SCHED_RR);
    slope = (max_prio_s - min_prio_s) / (float)(max_rr_prio - min_rr_prio);
    
    return min_prio_s + slope * (prio - min_rr_prio);
}

/**
 * @brief Given pointer to plugin struct @p this, retrieve least loaded cpu
 */
uint32_t least_loaded_cpu(struct rts_plugin* this)
{
    int cpu_num;
    int num_of_rr_min;
    int num_of_rr_min_cpu;

    num_of_rr_min_cpu = this->cpulist[0];
    num_of_rr_min = this->task_count_percpu[num_of_rr_min_cpu];

    for(int i = 1; i < this->cpunum; i++)
    {
        cpu_num = this->cpulist[i];

        if (this->task_count_percpu[cpu_num] < num_of_rr_min)
            num_of_rr_min_cpu = cpu_num;
    }

    return num_of_rr_min_cpu;
}

uint8_t has_another_preference(struct rts_plugin* this, struct rts_task* t)
{
    char* preferred = rts_task_get_preferred_plugin(t);

    if (preferred != NULL && strcmp(this->name, preferred) != 0)
        return 1;

    return 0;
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
    if (!rts_task_get_ignore_admission(t) && rts_task_get_priority(t) == 0)
        return RTS_PARTIAL;
    
    if (has_another_preference(this, t))
        return RTS_PARTIAL;

    return RTS_OK;
}

/**
 * @brief Used by plugin to perform a new admission test when task modifies parameters
 */
int rts_plg_task_change(struct rts_plugin* this, struct rts_taskset* ts, struct rts_task* t) 
{
    return rts_plg_task_accept(this, ts, t);
}

/**
 * @brief Used by plugin to set the task as accepted
 */
void rts_plg_task_schedule(struct rts_plugin* this, struct rts_taskset* ts, struct rts_task* t) 
{
    uint32_t priority;

    priority = rts_task_get_priority(t);
    rts_task_set_cpu(t, least_loaded_cpu(this));
    t->pluginid = this->id;

    if (priority == 0)
        rts_task_set_real_priority(t, this->prio_min);
    else
        rts_task_set_real_priority(t, prio_remap(this->prio_max, this->prio_min, priority));

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
    
    if(sched_setscheduler(t->tid, SCHED_RR, &attr) < 0)
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
    this->task_count_percpu[t->cpu]--;
    t->pluginid = -1;

    if (sched_getscheduler(t->tid) != SCHED_RR) // means no attached flow of ex.
        return RTS_OK;
    
    return rts_plg_task_detach(t);
}