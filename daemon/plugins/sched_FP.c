#define _GNU_SOURCE

#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "retif_taskset.h"
#include "retif_types.h"
#include "retif_utils.h"

// -----------------------------------------------------------------------------
// UTILITY INTERNAL METHODS
// -----------------------------------------------------------------------------

/**
 * @brief Given min/max plugin prio, normalize @p prio in that window
 */
uint32_t prio_remap(uint32_t max_prio_s, uint32_t min_prio_s, uint32_t prio)
{
    float slope;
    int max_fp_prio;
    int min_fp_prio;

    max_fp_prio = sched_get_priority_max(SCHED_FIFO);
    min_fp_prio = sched_get_priority_min(SCHED_FIFO);
    slope = (max_prio_s - min_prio_s) / (float)(max_fp_prio - min_fp_prio);

    return min_prio_s + slope * (prio - min_fp_prio);
}

/**
 * @brief Given pointer to plugin struct @p this, retrieve least loaded cpu
 */
uint32_t least_loaded_cpu(struct retif_plugin* this)
{
    int cpu_num;
    int num_of_fp_min;
    int num_of_fp_min_cpu;

    num_of_fp_min_cpu = this->cpulist[0];
    num_of_fp_min = this->task_count_percpu[num_of_fp_min_cpu];

    for(int i = 1; i < this->cputot; i++)
    {
        cpu_num = this->cpulist[i];

        if (this->task_count_percpu[cpu_num] < num_of_fp_min)
            num_of_fp_min_cpu = cpu_num;
    }

    return num_of_fp_min_cpu;
}

uint8_t has_another_preference(struct retif_plugin* this, struct retif_task* t)
{
    char* preferred = retif_task_get_preferred_plugin(t);

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
int retif_plg_task_init(struct retif_plugin* this)
{
    return retif_OK;
}

/**
 * @brief Used by plugin to perform a new task admission test
 */
int retif_plg_task_accept(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    if (!retif_task_get_ignore_admission(t) && retif_task_get_priority(t) == 0)
        return retif_PARTIAL;

    if (has_another_preference(this, t))
        return retif_PARTIAL;

    return retif_OK;
}

/**
 * @brief Used by plugin to perform a new admission test when task modifies parameters
 */
int retif_plg_task_change(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    return retif_plg_task_accept(this, ts, t);
}

/**
 * @brief Used by plugin to set the task as accepted
 */
void retif_plg_task_schedule(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    uint32_t priority;

    priority = retif_task_get_priority(t);
    retif_task_set_cpu(t, least_loaded_cpu(this));
    t->pluginid = this->id;

    if (priority == 0)
        retif_task_set_real_priority(t, this->prio_min);
    else
        retif_task_set_real_priority(t, prio_remap(this->prio_max, this->prio_min, priority));

    this->task_count_percpu[t->cpu]++;
}

/**
 * @brief Used by plugin to set rt scheduler for a task
 */
int retif_plg_task_attach(struct retif_task* t)
{
    struct sched_param attr;
    cpu_set_t my_set;

    CPU_ZERO(&my_set);
    CPU_SET(t->cpu, &my_set);

    if(sched_setaffinity(t->tid, sizeof(cpu_set_t), &my_set) < 0)
        return retif_ERROR;

    attr.sched_priority = t->schedprio;

    if(sched_setscheduler(t->tid, SCHED_FIFO, &attr) < 0)
        return retif_ERROR;

    return retif_OK;
}

/**
 * @brief Used by plugin to reset scheduler (other) for a task
 */
int retif_plg_task_detach(struct retif_task* t)
{
    struct sched_param attr;
    cpu_set_t my_set;

    CPU_ZERO(&my_set);

    for(int i = 0; i < get_nprocs2(); i++)
        CPU_SET(i, &my_set);

    if(sched_setaffinity(t->tid, sizeof(cpu_set_t), &my_set) < 0)
        return retif_ERROR;

    attr.sched_priority = 0;

    if(sched_setscheduler(t->tid, SCHED_OTHER, &attr) < 0)
        return retif_ERROR;

    return retif_OK;
}

/**
 * @brief Used by plugin to perform a release of previous accepted task
 */
int retif_plg_task_release(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    this->task_count_percpu[t->cpu]--;
    t->pluginid = -1;

    if (sched_getscheduler(t->tid) != SCHED_FIFO) // means no attached flow of ex.
        return retif_OK;

    return retif_plg_task_detach(t);
}
