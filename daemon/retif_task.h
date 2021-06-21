//-----------------------------------------------------------------------------
// RT_TASK_H: REPRESENT A REAL TIME TASK WITH ITS PARAM
//-----------------------------------------------------------------------------

#ifndef RETIF_TASK_H
#define RETIF_TASK_H

#include "retif_plugin.h"
#include "retif_types.h"
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

#define LOW_PRIO 1 // lowest RT priority
#define HIGH_PRIO 99 // highest RT priority

enum PARAM
{
    RUNTIME,
    PERIOD,
    DEADLINE,
    PRIORITY
};

#define ASC 1
#define DSC -1

struct rtf_task
{
    rtf_id_t id; /** task id in the system */
    pid_t ptid; /** parent tid */
    pid_t tid; /** thread/process id */
    uid_t euid; /** effective user id */
    gid_t egid; /** effective group id */
    clockid_t clk; /** type of clock to be used [REALTIME, MONOTONIC, ...] */
    uint64_t cpu; /** current cpu */
    uint32_t schedprio; /** scheduling real prio [LOW_PRIO, HIGH_PRIO] */
    int pluginid; /** if != -1 -> the scheduling alg */
    uint64_t acceptedt; /** accepted runtime */
    float acceptedu; /** accepted utils */
    struct rtf_params params;
};

//------------------------------------------
// PUBLIC: CREATE AND DESTROY FUNCTIONS
//------------------------------------------

// Instanciate and initialize a real time task structure
int rtf_task_init(struct rtf_task **t, rtf_id_t id, clockid_t clk);

// Instanciate and initialize a real time task structure from another one
int rtf_task_copy(struct rtf_task *t, struct rtf_task *t_copy);

// Destroy a real time task structure
void rtf_task_release(struct rtf_task *t);

//-----------------------------------------------
// PUBLIC: GETTER/SETTER
//------------------------------------------------

// Get the task cpu
uint32_t rtf_task_get_cpu(struct rtf_task *t);

// Set the task cpu
void rtf_task_set_cpu(struct rtf_task *t, uint32_t cpu);

// Get the task runtime
uint64_t rtf_task_get_runtime(struct rtf_task *t);

// Get the task desired runtime
uint64_t rtf_task_get_des_runtime(struct rtf_task *t);

// Get the task accepted runtime
uint64_t rtf_task_get_accepted_runtime(struct rtf_task *t);

// Set the task accepted runtime
void rtf_task_set_accepted_runtime(struct rtf_task *t, uint64_t runtime);

// Get the task period
uint64_t rtf_task_get_period(struct rtf_task *t);

// Get the relative deadline
uint64_t rtf_task_get_deadline(struct rtf_task *t);

// Get the declared priority
uint32_t rtf_task_get_priority(struct rtf_task *t);

// Get the real priority
uint32_t rtf_task_get_real_priority(struct rtf_task *t);

// Set the real priority
void rtf_task_set_real_priority(struct rtf_task *t, uint32_t priority);

// Get task minimum declared value among period and deadline
uint64_t rtf_task_get_min_declared(struct rtf_task *t);

// Get the task cpu utilization
float rtf_task_get_util(struct rtf_task *t);

// Get the task desired cpu utilization
float rtf_task_get_des_util(struct rtf_task *t);

// Get task ignore admission param
uint8_t rtf_task_get_ignore_admission(struct rtf_task *t);

// Get task preference plugin name
char *rtf_task_get_preferred_plugin(struct rtf_task *t);

// Compare two tasks
int task_cmp(struct rtf_task *t1, struct rtf_task *t2, enum PARAM p, int flag);

#endif /** RETIF_TASK_H */
