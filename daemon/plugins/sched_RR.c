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

#define MAX_CPU 32
#define MAX_PRIO 100

#define GET_BIT_VAL(bitval, bitpos) (((0x1 << bitpos) & bitval) >> bitpos)
#define SET_BIT_VAL(bitval, bitpos) ((0x1 << bitpos) | bitval)
struct priorities
{
    uint64_t low;
    uint64_t high;
};

static int n_per_prio[MAX_CPU][MAX_PRIO] = { 0 }; // TODO this will be dynamic
static unsigned int dist_prio[MAX_CPU] = { 0 };

unsigned int is_prio_unique(struct priorities* list, unsigned int priority)
{
    int bit_value;

    if (priority > 63)
        bit_value = GET_BIT_VAL(list->high, (priority - 63));
    else
        bit_value = GET_BIT_VAL(list->low, priority);

    return (bit_value == 0);
}

void set_prio_unique(struct priorities* list, unsigned int priority)
{
    if (priority > 63)
        list->high = SET_BIT_VAL(list->high, (priority - 63));
    else
        list->low = SET_BIT_VAL(list->low, priority);
}

/*
static int count_unique_priorities(struct retif_plugin* this, struct retif_taskset* ts, unsigned int cpu, struct retif_taskset* rr_tasks)
{
    struct retif_task* t_rr;
    iterator_t iterator;
    unsigned int dist_prio;
    struct priorities prio_list;

    dist_prio = 0;
    memset(&prio_list, 0, sizeof(prio_list));

    iterator = retif_taskset_iterator_init(ts);

    for (; iterator != NULL; iterator = iterator_get_next(iterator))
    {
        t_rr = retif_taskset_iterator_get_elem(iterator);

        if (t_rr->cpu != cpu || t_rr->pluginid != this->id)
            continue;

        retif_taskset_add_sorted_prio(rr_tasks, t_rr);

        if (is_prio_unique(&prio_list, t_rr->params.priority))
        {
            dist_prio++;
            set_prio_unique(&prio_list, t_rr->params.priority);
        }
    }

    return dist_prio;
}
*/

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

    slope = (max_prio_s - min_prio_s + 1) / (float)(max_rr_prio - min_rr_prio + 1);

    return min_prio_s + slope * (prio - min_rr_prio);
}

/**
 * @brief Given pointer to plugin struct @p this, retrieve least loaded cpu
 */
uint32_t least_loaded_cpu(struct retif_plugin* this)
{
    int cpu_num;
    int num_of_rr_min;
    int num_of_rr_min_cpu;

    num_of_rr_min_cpu = this->cpulist[0];
    num_of_rr_min = this->task_count_percpu[num_of_rr_min_cpu];

    for(int i = 1; i < this->cputot; i++)
    {
        cpu_num = this->cpulist[i];

        if (this->task_count_percpu[cpu_num] < num_of_rr_min)
            num_of_rr_min_cpu = cpu_num;
    }

    return num_of_rr_min_cpu;
}

static void assign_priorities(struct retif_plugin* this, unsigned int cpu)
{
    unsigned int curr_prio; // real prio
    unsigned int prec_prio; // user prio

    iterator_t iterator;
    struct retif_task* t_rr;

    curr_prio = this->prio_min-1;
    prec_prio = -1;

    iterator = retif_taskset_iterator_init(&this->tasks[cpu]);

    if (dist_prio[cpu] > (this->prio_max - this->prio_min) + 1)
    {
        for (; iterator != NULL; iterator = iterator_get_next(iterator))
        {
            t_rr = retif_taskset_iterator_get_elem(iterator);
            retif_task_set_real_priority(t_rr,
                prio_remap(this->prio_max, this->prio_min, t_rr->params.priority));
        }
    }
    else
    {
        for (; iterator != NULL; iterator = iterator_get_next(iterator))
        {
            t_rr = retif_taskset_iterator_get_elem(iterator);

            if (prec_prio == retif_task_get_priority(t_rr))
            {
                retif_task_set_real_priority(t_rr, curr_prio);
            }
            else
            {
                retif_task_set_real_priority(t_rr, ++curr_prio);
                prec_prio = retif_task_get_priority(t_rr);
            }
        }

    }

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
    return RETIF_OK;
}

/**
 * @brief Used by plugin to perform a new task admission test
 */
int retif_plg_task_accept(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    if (!retif_task_get_ignore_admission(t) && retif_task_get_priority(t) == 0)
        return RETIF_PARTIAL;

    if (has_another_preference(this, t))
        return RETIF_PARTIAL;

    return RETIF_OK;
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
    unsigned int cpu = least_loaded_cpu(this);

    retif_task_set_cpu(t, cpu);

    t->pluginid = this->id;

    n_per_prio[cpu][t->params.priority]++;

    struct node_ptr* inserted =
        retif_taskset_add_sorted_prio(&this->tasks[cpu], t);

    if (n_per_prio[cpu][t->params.priority] == 1)
    {
        dist_prio[cpu]++;
        assign_priorities(this, cpu);
    }
    else
    {
        // if (inserted->next == NULL)
        //     retif_task_set_real_priority(t, this->prio_max);
        // else
            retif_task_set_real_priority(t, retif_task_get_real_priority(
                (struct retif_task*)inserted->next->elem));
    }

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
        return RETIF_ERROR;

    attr.sched_priority = t->schedprio;

    if(sched_setscheduler(t->tid, SCHED_RR, &attr) < 0)
        return RETIF_ERROR;

    return RETIF_OK;
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
        return RETIF_ERROR;

    attr.sched_priority = 0;

    if(sched_setscheduler(t->tid, SCHED_OTHER, &attr) < 0)
        return RETIF_ERROR;

    return RETIF_OK;
}

/**
 * @brief Used by plugin to perform a release of previous accepted task
 */
int retif_plg_task_release(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    this->task_count_percpu[t->cpu]--;

    n_per_prio[t->cpu][t->params.priority]--;

    if (n_per_prio[t->cpu][t->params.priority] == 0)
        dist_prio[t->cpu]--;

    retif_taskset_remove_by_rsvid(&this->tasks[t->cpu], t->id);
    t->pluginid = -1;

    if (sched_getscheduler(t->tid) != SCHED_RR) // means no attached flow of ex.
        return RETIF_OK;

    return retif_plg_task_detach(t);
}
