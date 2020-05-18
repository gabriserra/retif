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
#include "rts_taskset.h"
#include "rts_plugin.h"
#include "rts_types.h"

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

#define MILLI_TO_NANO(var) var * 1000 * 1000
#define MICRO_TO_NANO(var) var * 1000

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
static uint32_t least_loaded_cpu(struct rts_plugin* this)
{
    int cpu_num;
    float free_edf_max;
    int free_edf_max_cpu;

    free_edf_max_cpu = this->cpulist[0];
    free_edf_max = this->util_free_percpu[free_edf_max_cpu];

    for(int i = 1; i < this->cpunum; i++)
    {
        cpu_num = this->cpulist[i];

        if (this->util_free_percpu[cpu_num] > free_edf_max)
            free_edf_max_cpu = cpu_num;
    }

    return free_edf_max_cpu;
}

static float eval_util_missing(struct rts_plugin* this, float task_util)
{
    int cpu_min;

    for(int i = 0; i < this->cpunum; i++)
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

static int desired_utilization_test(struct rts_plugin* this, float task_util, float task_des_util)
{
    float missing_util = eval_util_missing(this, task_util);
    float missing_des_util = eval_util_missing(this, task_des_util);

    if (missing_des_util == 0)
        return RTS_OK;
    else if (missing_des_util > 0 && missing_util == 0)
        return RTS_PARTIAL;
    else
        return RTS_NO;
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
    float task_des_util;
    int test_res;

    task_util = rts_task_get_util(t);
    task_des_util = rts_task_get_des_util(t);

    // task does not have required params
    if (rts_task_get_ignore_admission(t))
        return RTS_NO;
    if (rts_task_get_period(t) == 0)
        return RTS_NO;
    if (task_util == -1)
        return RTS_NO;
    
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
    if (t->pluginid == this->id)
        this->util_free_percpu[t->cpu] += t->acceptedu;

    test_res = rts_plg_task_accept(this, ts, t);

    // restore utilization
    if (t->pluginid == this->id)
        this->util_free_percpu[t->cpu] -= t->acceptedu;

    return test_res;
}

/**
 * @brief Used by plugin to set the task as accepted
 */
void rts_plg_task_schedule(struct rts_plugin* this, struct rts_taskset* ts, struct rts_task* t) 
{
    float task_util;
    float task_des_util;

    task_util = rts_task_get_util(t);
    task_des_util = rts_task_get_des_util(t);

    t->cpu = least_loaded_cpu(this);
    t->pluginid = this->id;

    // task does not require a desired higher runtime
    if (task_des_util == -1)
    {
        t->acceptedt = rts_task_get_runtime(t);
        t->acceptedu = task_util;
    }
    // required higher desired runtime and it is available
    else if (desired_utilization_test(this, task_util, task_des_util) == RTS_OK)
    {
        t->acceptedt = rts_task_get_des_runtime(t);
        t->acceptedu = task_util;
    }
    // required higher desired runtime but not available all
    else
    {
        t->acceptedt = this->util_free_percpu[t->cpu] * rts_task_get_min_declared(t);
        t->acceptedu = t->acceptedt / (float)rts_task_get_min_declared(t);
    }

    this->util_free_percpu[t->cpu] -= t->acceptedu;
}

/**
 * @brief Used by plugin to set rt scheduler for a task
 */
int rts_plg_task_attach(struct rts_task* t) 
{
    struct sched_attr attr;
    cpu_set_t my_set;
    uint64_t runtime;
    uint64_t deadline;
    uint64_t period;

    CPU_ZERO(&my_set);
    CPU_SET(t->cpu, &my_set);

    if(sched_setaffinity(t->tid, sizeof(cpu_set_t), &my_set) < 0)
        return RTS_ERROR;

    runtime     = rts_task_get_accepted_runtime(t);
    deadline    = rts_task_get_deadline(t) != 0 ? rts_task_get_deadline(t) : rts_task_get_period(t);
    period      = rts_task_get_period(t);

    memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(attr);

    attr.sched_policy   = SCHED_DEADLINE;
    attr.sched_runtime  = MICRO_TO_NANO(runtime);
    attr.sched_deadline = MICRO_TO_NANO(deadline);
    attr.sched_period   = MICRO_TO_NANO(period);
        
    if(sched_setattr(t->tid, &attr, 0) < 0)
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
    this->util_free_percpu[t->cpu] += t->acceptedu;
    t->pluginid = -1;

    if (sched_getscheduler(t->tid) != SCHED_DEADLINE) // means no attached flow of ex.
        return RTS_OK;
    
    return rts_plg_task_detach(t);
}