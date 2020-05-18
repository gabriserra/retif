#ifndef RTS_LIB_H
#define RTS_LIB_H

#include <stdint.h>
#include <time.h>
#include <sched.h>

// -----------------------------------------------------------------------------
// PLUGINS INFO
// -----------------------------------------------------------------------------

#define PLUGIN_MAX_NAME         10

// -----------------------------------------------------------------------------
// TIME UTILS (MACRO - TYPES)
// -----------------------------------------------------------------------------

#define EXP3 1000
#define EXP6 1000000
#define EXP9 1000000000

#define SEC_TO_MILLI(sec) sec * EXP3
#define SEC_TO_MICRO(sec) sec * EXP6
#define SEC_TO_NANO(sec) sec * EXP9

#define MILLI_TO_SEC(milli) milli / EXP3
#define MILLI_TO_MICRO(milli) milli * EXP3
#define MILLI_TO_NANO(milli) milli * EXP6

#define MICRO_TO_SEC(micro) micro / EXP6
#define MICRO_TO_MILLI(micro) micro / EXP3
#define MICRO_TO_NANO(micro) micro * EXP3

#define NANO_TO_SEC(nano) nano / EXP9
#define NANO_TO_MILLI(nano) nano / EXP6
#define NANO_TO_MICRO(nano) nano / EXP3

// -----------------------------------------------------------------------------
// STRUCT UTILS
// -----------------------------------------------------------------------------

#ifndef RTS_PUBLIC_TYPES
#define RTS_PUBLIC_TYPES

struct rts_params
{
    uint64_t    runtime;                        // required runtime [microseconds]
    uint64_t    des_runtime;                    // desired runtime [microseconds]
    uint64_t    period;                         // period of task [microseconds]
    uint64_t    deadline;                       // relative deadline [microseconds]
    uint32_t    priority;                       // priority of task [LOW_PRIO, HIGH_PRIO]
    char        sched_plugin[PLUGIN_MAX_NAME];  // preferenced plugin to be used
    uint8_t     ignore_admission;               // preference to avoid test
};

struct rts_task
{
    uint32_t            task_id;        // task id assigned by daemon
    uint32_t            dmiss;		    // num of deadline misses
    uint64_t            acc_runtime;    // accepted runtime
    struct rts_access*  c;              // channel used to communicate with daemon
    struct rts_params   p;              // preferred scheduling parameters
    struct timespec     at;	            // next activation time 
    struct timespec     dl; 	        // absolute deadline
};

#endif

// -----------------------------------------------------------------------------
// GETTER / SETTER
// -----------------------------------------------------------------------------

void rts_params_init(struct rts_params *p);

void rts_params_set_runtime(struct rts_params* p, uint64_t runtime);

uint64_t rts_params_get_runtime(struct rts_params* p);

void rts_params_set_des_runtime(struct rts_params* p, uint64_t runtime);

uint64_t rts_params_get_des_runtime(struct rts_params* p);

void rts_params_set_period(struct rts_params* p, uint64_t period);

uint64_t rts_params_get_period(struct rts_params* p);

void rts_params_set_deadline(struct rts_params*p, uint64_t deadline);

uint64_t rts_params_get_deadline(struct rts_params* p);

void rts_params_set_priority(struct rts_params* p, uint32_t priority);

uint32_t rts_params_get_priority(struct rts_params* p);

void rts_params_set_scheduler(struct rts_params* p, char sched_plugin[PLUGIN_MAX_NAME]);

void rts_params_get_scheduler(struct rts_params* p, char sched_plugin[PLUGIN_MAX_NAME]);

void rts_params_ignore_admission(struct rts_params* p, uint8_t ignore_admission);

// -----------------------------------------------------------------------------
// COMMUNICATION WITH DAEMON
// -----------------------------------------------------------------------------

int rts_daemon_connect();

void rts_task_init(struct rts_task* t);

int rts_task_create(struct rts_task* t, struct rts_params* p);

int rts_task_change(struct rts_task* t, struct rts_params* p);

int rts_task_attach(struct rts_task* t, pid_t pid);

int rts_task_detach(struct rts_task* t);

int rts_task_destroy(struct rts_task* t);

// -----------------------------------------------------------------------------
// LOCAL COMMUNICATIONS
// -----------------------------------------------------------------------------

void rts_task_start(struct rts_task* t);

void rts_task_wait_period(struct rts_task* t);

uint64_t rts_task_get_accepted_runtime(struct rts_task* t);

#endif	// RTS_LIB_H

