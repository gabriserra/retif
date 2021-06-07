#ifndef retif_LIB_H
#define retif_LIB_H

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

#ifndef retif_PUBLIC_TYPES
#define retif_PUBLIC_TYPES

struct retif_params
{
    uint64_t    runtime;                        // required runtime [microseconds]
    uint64_t    des_runtime;                    // desired runtime [microseconds]
    uint64_t    period;                         // period of task [microseconds]
    uint64_t    deadline;                       // relative deadline [microseconds]
    uint32_t    priority;                       // priority of task [LOW_PRIO, HIGH_PRIO]
    char        sched_plugin[PLUGIN_MAX_NAME];  // preferenced plugin to be used
    uint8_t     ignore_admission;               // preference to avoid test
};

struct retif_task
{
    uint32_t            task_id;        // task id assigned by daemon
    uint32_t            dmiss;		    // num of deadline misses
    uint64_t            acc_runtime;    // accepted runtime
    struct retif_access*  c;              // channel used to communicate with daemon
    struct retif_params   p;              // preferred scheduling parameters
    struct timespec     at;	            // next activation time
    struct timespec     dl; 	        // absolute deadline
};

#endif

// -----------------------------------------------------------------------------
// GETTER / SETTER
// -----------------------------------------------------------------------------

void retif_params_init(struct retif_params *p);

void retif_params_set_runtime(struct retif_params* p, uint64_t runtime);

uint64_t retif_params_get_runtime(struct retif_params* p);

void retif_params_set_des_runtime(struct retif_params* p, uint64_t runtime);

uint64_t retif_params_get_des_runtime(struct retif_params* p);

void retif_params_set_period(struct retif_params* p, uint64_t period);

uint64_t retif_params_get_period(struct retif_params* p);

void retif_params_set_deadline(struct retif_params*p, uint64_t deadline);

uint64_t retif_params_get_deadline(struct retif_params* p);

void retif_params_set_priority(struct retif_params* p, uint32_t priority);

uint32_t retif_params_get_priority(struct retif_params* p);

void retif_params_set_scheduler(struct retif_params* p, char sched_plugin[PLUGIN_MAX_NAME]);

void retif_params_get_scheduler(struct retif_params* p, char sched_plugin[PLUGIN_MAX_NAME]);

void retif_params_ignore_admission(struct retif_params* p, uint8_t ignore_admission);

// -----------------------------------------------------------------------------
// COMMUNICATION WITH DAEMON
// -----------------------------------------------------------------------------

int retif_daemon_connect();

void retif_task_init(struct retif_task* t);

int retif_task_create(struct retif_task* t, struct retif_params* p);

int retif_task_change(struct retif_task* t, struct retif_params* p);

int retif_task_attach(struct retif_task* t, pid_t pid);

int retif_task_detach(struct retif_task* t);

int retif_task_destroy(struct retif_task* t);

// -----------------------------------------------------------------------------
// LOCAL COMMUNICATIONS
// -----------------------------------------------------------------------------

void retif_task_start(struct retif_task* t);

void retif_task_wait_period(struct retif_task* t);

uint64_t retif_task_get_accepted_runtime(struct retif_task* t);

#endif	// retif_LIB_H
