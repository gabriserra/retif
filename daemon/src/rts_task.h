//-----------------------------------------------------------------------------
// RT_TASK_H: REPRESENT A REAL TIME TASK WITH ITS PARAM
//----------------------------------------------------------------------------- 

#ifndef RTS_TASK_H
#define RTS_TASK_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include "rts_types.h"
#include "rts_plugin.h"

#define LOW_PRIO 	1			// lowest RT priority
#define HIGH_PRIO	99			// highest RT priority

enum PARAM 
{
    RUNTIME,
    PERIOD,
    DEADLINE,
    PRIORITY
};

#define ASC 1
#define DSC -1

struct rts_task 
{
    rts_id_t            id;         /** task id in the system */
    pid_t               ptid;		/** parent tid */
    pid_t               tid;		/** thread/process id */
    clockid_t           clk;        /** type of clock to be used [REALTIME, MONOTONIC, ...] */
    uint64_t            cpu;        /** current cpu */
    uint32_t            schedprio;  /** scheduling real prio [LOW_PRIO, HIGH_PRIO] */
    int                 pluginid;   /** if != -1 -> the scheduling alg */
    uint64_t            acceptedt;  /** accepted runtime */
    float               acceptedu;  /** accepted utils */
    struct rts_params   params;
};


//------------------------------------------
// PUBLIC: CREATE AND DESTROY FUNCTIONS
//------------------------------------------

// Instanciate and initialize a real time task structure
int rts_task_init(struct rts_task **t, rts_id_t id, clockid_t clk);

// Instanciate and initialize a real time task structure from another one
int rts_task_copy(struct rts_task *t, struct rts_task *t_copy);

// Destroy a real time task structure
void rts_task_destroy(struct rts_task *t);

//-----------------------------------------------
// PUBLIC: GETTER/SETTER
//------------------------------------------------

// Get the task cpu
uint32_t rts_task_get_cpu(struct rts_task* t);

// Set the task cpu
void rts_task_set_cpu(struct rts_task* t, uint32_t cpu);

// Get the task runtime
uint64_t rts_task_get_runtime(struct rts_task* t);

// Get the task desired runtime
uint64_t rts_task_get_des_runtime(struct rts_task* t);

// Get the task accepted runtime
uint64_t rts_task_get_accepted_runtime(struct rts_task* t);

// Set the task accepted runtime
void rts_task_set_accepted_runtime(struct rts_task* t, uint64_t runtime);

// Get the task period
uint64_t rts_task_get_period(struct rts_task* t);

// Get the relative deadline
uint64_t rts_task_get_deadline(struct rts_task* t);

// Get the declared priority
uint32_t rts_task_get_priority(struct rts_task* t);

// Get the real priority
uint32_t rts_task_get_real_priority(struct rts_task* t);

// Set the real priority
void rts_task_set_real_priority(struct rts_task* t, uint32_t priority);

// Get task minimum declared value among period and deadline
uint64_t rts_task_get_min_declared(struct rts_task* t);

// Get the task cpu utilization
float rts_task_get_util(struct rts_task* t);

// Get the task desired cpu utilization
float rts_task_get_des_util(struct rts_task* t);

// Get task ignore admission param
uint8_t rts_task_get_ignore_admission(struct rts_task* t);

// Get task preference plugin name
char* rts_task_get_preferred_plugin(struct rts_task* t);

// Compare two tasks
int task_cmp(struct rts_task* t1, struct rts_task* t2, enum PARAM p, int flag) ;

#endif /** RTS_TASK_H */
