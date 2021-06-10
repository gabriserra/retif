#define _GNU_SOURCE

#include <sched.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include "retif_taskset.h"
#include "retif_plugin.h"
#include "retif_types.h"
#include "retif_utils.h"

// -----------------------------------------------------------------------------
// LIBC FOR SCHED_DEADLINE
// -----------------------------------------------------------------------------

// unfortunately on many distros is not present the libc for SCHED_DEADLINE
// what follows may be removed in future

#ifdef __x86_64__
    #define __NR_sched_setattr		314
    #define __NR_sched_getattr		315
#endif

#ifdef __i386__
    #define __NR_sched_setattr		351
    #define __NR_sched_getattr		352
#endif

#ifdef __arm__
    #define __NR_sched_setattr		380
    #define __NR_sched_getattr		381
#endif

struct sched_attr
{
	__u32 size;
	__u32 sched_policy;
	__u64 sched_flags;
	__s32 sched_nice;
	__u32 sched_priority;
	__u64 sched_runtime;
	__u64 sched_deadline;
    __u64 sched_period;
};

/**
 * @brief Sets the scheduling policy and attributes for the thread specified
 */
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

// -----------------------------------------------------------------------------
// UTILITY INTERNAL METHODS
// -----------------------------------------------------------------------------

/**
 * @brief Given pointer to plugin struct @p this, retrieve least loaded cpu
 */
static uint32_t least_loaded_cpu(struct retif_plugin* this)
{
    int cpu_num;
    float free_edf_max;
    int free_edf_max_cpu;

    free_edf_max_cpu = this->cpulist[0];
    free_edf_max = this->util_free_percpu[free_edf_max_cpu];

    for(int i = 1; i < this->cputot; i++)
    {
        cpu_num = this->cpulist[i];

        if (this->util_free_percpu[cpu_num] > free_edf_max)
            free_edf_max_cpu = cpu_num;
    }

    return free_edf_max_cpu;
}

static float eval_util_missing(struct retif_plugin* this, float task_util)
{
    int cpu_min;

    for(int i = 0; i < this->cputot; i++)
        if(task_util <= this->util_free_percpu[this->cpulist[i]])
            return 0;

    cpu_min = least_loaded_cpu(this);

    return task_util - this->util_free_percpu[cpu_min];
}

static uint8_t has_another_preference(struct retif_plugin* this, struct retif_task* t)
{
    char* preferred = retif_task_get_preferred_plugin(t);

    if (preferred != NULL && strcmp(this->name, preferred) != 0)
        return 1;

    return 0;
}

static int utilization_test(struct retif_plugin* this, float task_util)
{
    float missing_util = eval_util_missing(this, task_util);

    if (missing_util == 0)
        return RETIF_OK;
    else
        return RETIF_NO;
}

static int desired_utilization_test(struct retif_plugin* this, float task_util, float task_des_util)
{
    float missing_util = eval_util_missing(this, task_util);
    float missing_des_util = eval_util_missing(this, task_des_util);

    if (missing_des_util == 0)
        return RETIF_OK;
    else if (missing_des_util > 0 && missing_util == 0)
        return RETIF_PARTIAL;
    else
        return RETIF_NO;
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
    float task_util;
    float task_des_util;
    int test_res;

    task_util = retif_task_get_util(t);
    task_des_util = retif_task_get_des_util(t);

    // task does not have required params
    if (retif_task_get_ignore_admission(t))
        return RETIF_NO;
    if (retif_task_get_period(t) == 0)
        return RETIF_NO;
    if (task_util == -1)
        return RETIF_NO;

    // task does not require a desired higher runtime
    if (task_des_util == -1)
    {
        test_res = utilization_test(this, task_util);
    }
    // task required a desired higher runtime
    else
    {
        test_res = desired_utilization_test(this, task_util, task_des_util);
    }

    // if not preferred plugin support is partial
    if (has_another_preference(this, t) && test_res == RETIF_OK)
        test_res = RETIF_PARTIAL;

    return test_res;
}

/**
 * @brief Used by plugin to perform a new admission test when task modifies parameters
 */
int retif_plg_task_change(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    int test_res;

    // simulate test without task utilization in
    if (t->pluginid == this->id)
        this->util_free_percpu[t->cpu] += t->acceptedu;

    test_res = retif_plg_task_accept(this, ts, t);

    // restore utilization
    if (t->pluginid == this->id)
        this->util_free_percpu[t->cpu] -= t->acceptedu;

    return test_res;
}

/**
 * @brief Used by plugin to set the task as accepted
 */
void retif_plg_task_schedule(struct retif_plugin* this, struct retif_taskset* ts, struct retif_task* t)
{
    float task_util;
    float task_des_util;

    task_util = retif_task_get_util(t);
    task_des_util = retif_task_get_des_util(t);

    t->cpu = least_loaded_cpu(this);
    t->pluginid = this->id;

    // task does not require a desired higher runtime
    if (task_des_util == -1)
    {
        t->acceptedt = retif_task_get_runtime(t);
        t->acceptedu = task_util;
    }
    // required higher desired runtime and it is available
    else if (desired_utilization_test(this, task_util, task_des_util) == RETIF_OK)
    {
        t->acceptedt = retif_task_get_des_runtime(t);
        t->acceptedu = task_util;
    }
    // required higher desired runtime but not available all
    else
    {
        t->acceptedt = this->util_free_percpu[t->cpu] * retif_task_get_min_declared(t);
        t->acceptedu = t->acceptedt / (float)retif_task_get_min_declared(t);
    }

    this->util_free_percpu[t->cpu] -= t->acceptedu;
    this->task_count_percpu[t->cpu]++;
}

/**
 * @brief Used by plugin to set rt scheduler for a task
 */
int retif_plg_task_attach(struct retif_task* t)
{
    struct sched_attr attr;
    cpu_set_t my_set;
    uint64_t runtime;
    uint64_t deadline;
    uint64_t period;

    CPU_ZERO(&my_set);
    CPU_SET(t->cpu, &my_set);

    if(sched_setaffinity(t->tid, sizeof(cpu_set_t), &my_set) < 0)
        return RETIF_ERROR;

    runtime     = retif_task_get_accepted_runtime(t);
    deadline    = retif_task_get_deadline(t) != 0 ? retif_task_get_deadline(t) : retif_task_get_period(t);
    period      = retif_task_get_period(t);

    memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(attr);

    attr.sched_policy   = SCHED_DEADLINE;
    attr.sched_runtime  = MICRO_TO_NANO(runtime);
    attr.sched_deadline = MICRO_TO_NANO(deadline);
    attr.sched_period   = MICRO_TO_NANO(period);

    if(sched_setattr(t->tid, &attr, 0) < 0)
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
    this->util_free_percpu[t->cpu] += t->acceptedu;
    this->task_count_percpu[t->cpu]--;
    t->pluginid = -1;

    if (sched_getscheduler(t->tid) != SCHED_DEADLINE) // means no attached flow of ex.
        return RETIF_OK;

    return retif_plg_task_detach(t);
}
