#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include <assert.h>
#include "retif_task.h"
#include "retif_utils.h"

#define _GNU_SOURCE

//------------------------------------------
// PRIVATE: UTILITIES FUNCTION
//------------------------------------------

//------------------------------------------
// PUBLIC: CREATE AND DESTROY FUNCTIONS
//------------------------------------------

// Instanciate and initialize a real time task structure
int rtf_task_init(struct rtf_task **t, rtf_id_t id, clockid_t clk)
{
    (*t) = calloc(1, sizeof(struct rtf_task));

    if((*t) == NULL)
        return -1;

    (*t)->id = id;
    (*t)->clk = clk;

    return 0;
}

// Instanciate and initialize a real time task structure from another one
int rtf_task_copy(struct rtf_task *t, struct rtf_task *t_copy)
{
    t = calloc(1, sizeof(struct rtf_task));

    if (t == NULL)
        return 0;

    memcpy(t, t_copy, sizeof(struct rtf_task));
    return 1;
}

// Destroy a real time task structure
void rtf_task_release(struct rtf_task *t)
{
    free(t);
}

//-----------------------------------------------
// PUBLIC: GETTER/SETTER
//------------------------------------------------

// Get the task cpu
uint32_t rtf_task_get_cpu(struct rtf_task* t)
{
    return t->cpu;
}

// Set the task cpu
void rtf_task_set_cpu(struct rtf_task* t, uint32_t cpu)
{
    t->cpu = cpu;
}

// Get the task runtime
uint64_t rtf_task_get_runtime(struct rtf_task* t)
{
    return t->params.runtime;
}

// Get the task desired runtime
uint64_t rtf_task_get_des_runtime(struct rtf_task* t)
{
    return t->params.des_runtime;
}

// Get the task accepted runtime
uint64_t rtf_task_get_accepted_runtime(struct rtf_task* t)
{
    return t->acceptedt;
}

// Set the task accepted runtime
void rtf_task_set_accepted_runtime(struct rtf_task* t, uint64_t runtime)
{
    t->acceptedt = runtime;
}

// Get the task period
uint64_t rtf_task_get_period(struct rtf_task* t)
{
    return t->params.period;
}

// Get the relative deadline
uint64_t rtf_task_get_deadline(struct rtf_task* t)
{
    return t->params.deadline;
}

// Get the declared priority
uint32_t rtf_task_get_priority(struct rtf_task* t)
{
    return t->params.priority;
}

// Get the real priority
uint32_t rtf_task_get_real_priority(struct rtf_task* t)
{
    return t->schedprio;
}

// Set the real priority
void rtf_task_set_real_priority(struct rtf_task* t, uint32_t priority)
{
    t->schedprio = priority;
}

// Get task ignore admission param
uint8_t rtf_task_get_ignore_admission(struct rtf_task* t)
{
    return t->params.ignore_admission;
}

// Get task preference plugin name
char* rtf_task_get_preferred_plugin(struct rtf_task* t)
{
    if (strcmp(t->params.sched_plugin, "\0") == 0)
        return NULL;

    return t->params.sched_plugin;
}

// Get task minimum declared value among period and deadline
uint64_t rtf_task_get_min_declared(struct rtf_task* t)
{
    if (t->params.period == 0)
        return t->params.deadline;
    if (t->params.deadline == 0)
        return t->params.period;

    return (t->params.deadline < t->params.period) ?
                        t->params.deadline : t->params.period;
}

// Get the task cpu utilization
float rtf_task_get_util(struct rtf_task* t)
{
    int min = rtf_task_get_min_declared(t);

    if (min == 0 || t->params.runtime == 0)
        return -1;

    return t->params.runtime / (float) min;
}

// Get the task desired cpu utilization
float rtf_task_get_des_util(struct rtf_task* t)
{
    int min = rtf_task_get_min_declared(t);

    if (min == 0 || t->params.des_runtime == 0)
        return -1;

    return t->params.des_runtime / (float) min;
}

//------------------------------------------
// PUBLIC: COMPARISON FUNCTIONS
//------------------------------------------

// Compare two tasks based on declared deadline
static int task_cmp_deadline(struct rtf_task* t1, struct rtf_task* t2)
{
    if(t1->params.deadline > t2->params.deadline)
        return 1;
    else if(t1->params.deadline < t2->params.deadline)
        return -1;
    else
        return 0;
}

// Compare two tasks based on declared period
static int task_cmp_period(struct rtf_task* t1, struct rtf_task* t2)
{
    if(t1->params.period > t2->params.period)
        return 1;
    else if(t1->params.period < t2->params.period)
        return -1;
    else
        return 0;
}

// Compare two tasks based on declared runtime
static int task_cmp_runtime(struct rtf_task* t1, struct rtf_task* t2)
{
    if(t1->params.runtime > t2->params.runtime)
        return 1;
    else if(t1->params.runtime < t2->params.runtime)
        return -1;
    else
        return 0;
}

// Compare two tasks based on declared priority
static int task_cmp_priority(struct rtf_task* t1, struct rtf_task* t2)
{
    if(t1->params.priority > t2->params.priority)
        return 1;
    else if(t1->params.priority < t2->params.priority)
        return -1;
    else
        return 0;
}

// Compare two tasks
int task_cmp(struct rtf_task* t1, struct rtf_task* t2, enum PARAM p, int flag)
{
    if(flag != ASC && flag != DSC)
        flag = ASC;

    switch (p)
    {
        case PERIOD:
            return flag * task_cmp_period(t1, t2);
        case RUNTIME:
            return flag * task_cmp_runtime(t1, t2);
        case DEADLINE:
            return flag * task_cmp_deadline(t1, t2);
        case PRIORITY:
            return flag * task_cmp_priority(t1, t2);
        default:
            return flag * task_cmp_priority(t1, t2);
    }
}
