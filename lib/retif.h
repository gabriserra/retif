#ifndef RETIF_LIB_H
#define RETIF_LIB_H

#include <sched.h>
#include <stdint.h>
#include <time.h>

// -----------------------------------------------------------------------------
// PLUGINS INFO
// -----------------------------------------------------------------------------

#define PLUGIN_MAX_NAME 32

// -----------------------------------------------------------------------------
// TIME UTILS (MACRO - TYPES)
// -----------------------------------------------------------------------------

#define EXP3 1000.0
#define EXP6 1000000.0
#define EXP9 1000000000.0

#define SEC_TO_MILLI(sec) sec *EXP3
#define SEC_TO_MICRO(sec) sec *EXP6
#define SEC_TO_NANO(sec) sec *EXP9

#define MILLI_TO_SEC(milli) milli / EXP3
#define MILLI_TO_MICRO(milli) milli *EXP3
#define MILLI_TO_NANO(milli) milli *EXP6

#define MICRO_TO_SEC(micro) micro / EXP6
#define MICRO_TO_MILLI(micro) micro / EXP3
#define MICRO_TO_NANO(micro) micro *EXP3

#define NANO_TO_SEC(nano) nano / EXP9
#define NANO_TO_MILLI(nano) nano / EXP6
#define NANO_TO_MICRO(nano) nano / EXP3

// -----------------------------------------------------------------------------
// STRUCT UTILS
// -----------------------------------------------------------------------------

#ifndef RETIF_PUBLIC_TYPES
#    define RETIF_PUBLIC_TYPES

#    define RTF_OK 1
#    define RTF_FAIL 0
#    define RTF_ERROR -1
#    define RTF_PARTIAL 0
#    define RTF_NO -1

struct rtf_params
{
    uint64_t runtime; // required runtime [microseconds]
    uint64_t des_runtime; // desired runtime [microseconds]
    uint64_t period; // period of task [microseconds]
    uint64_t deadline; // relative deadline [microseconds]
    uint32_t priority; // priority of task [LOW_PRIO, HIGH_PRIO]
    char sched_plugin[PLUGIN_MAX_NAME]; // preferenced plugin to be used
    uint8_t ignore_admission; // preference to avoid test
};

static const struct rtf_params RTF_PARAM_INIT = {0};

struct rtf_task
{
    uint32_t task_id; // task id assigned by daemon
    uint32_t dmiss; // num of deadline misses
    uint64_t acc_runtime; // accepted runtime
    struct rtf_access *c; // channel used to communicate with daemon
    struct rtf_params p; // preferred scheduling parameters
    struct timespec at; // next activation time
    struct timespec dl; // absolute deadline
};

static const struct rtf_task RTF_TASK_INIT = {0};

struct rtf_client_info
{
    pid_t pid;
    int state;
};
struct rtf_task_info
{
    pid_t tid;
    pid_t ppid;
    int priority;
    int period;
    float util;
    int pluginid;
};
struct rtf_plugin_info
{
    char name[PLUGIN_MAX_NAME];
    int cpunum;
};
struct rtf_cpu_info
{
    int cpunum;
    float freeu;
    int ntask;
};

#endif

// -----------------------------------------------------------------------------
// GETTER / SETTER
// -----------------------------------------------------------------------------

void rtf_params_init(struct rtf_params *p);

void rtf_params_set_runtime(struct rtf_params *p, uint64_t runtime);

uint64_t rtf_params_get_runtime(struct rtf_params *p);

void rtf_params_set_des_runtime(struct rtf_params *p, uint64_t runtime);

uint64_t rtf_params_get_des_runtime(struct rtf_params *p);

void rtf_params_set_period(struct rtf_params *p, uint64_t period);

uint64_t rtf_params_get_period(struct rtf_params *p);

void rtf_params_set_deadline(struct rtf_params *p, uint64_t deadline);

uint64_t rtf_params_get_deadline(struct rtf_params *p);

void rtf_params_set_priority(struct rtf_params *p, uint32_t priority);

uint32_t rtf_params_get_priority(struct rtf_params *p);

void rtf_params_set_scheduler(struct rtf_params *p,
    char sched_plugin[PLUGIN_MAX_NAME]);

void rtf_params_get_scheduler(struct rtf_params *p,
    char sched_plugin[PLUGIN_MAX_NAME]);

void rtf_params_ignore_admission(struct rtf_params *p,
    uint8_t ignore_admission);

// -----------------------------------------------------------------------------
// COMMUNICATION WITH DAEMON
// -----------------------------------------------------------------------------

int rtf_connect();

int rtf_connections_info();

int rtf_plugins_info();

int rtf_tasks_info();

int rtf_connection_info(unsigned int desc, struct rtf_client_info *data);

int rtf_task_info(unsigned int desc, struct rtf_task_info *data);

int rtf_plugin_info(unsigned int desc, struct rtf_plugin_info *data);

int rtf_plugin_cpu_info(unsigned int desc, unsigned int cpuid,
    struct rtf_cpu_info *data);

void rtf_task_init(struct rtf_task *t);

int rtf_task_create(struct rtf_task *t, struct rtf_params *p);

int rtf_task_change(struct rtf_task *t, struct rtf_params *p);

int rtf_task_attach(struct rtf_task *t, pid_t pid);

int rtf_task_detach(struct rtf_task *t);

int rtf_task_release(struct rtf_task *t);

// -----------------------------------------------------------------------------
// LOCAL COMMUNICATIONS
// -----------------------------------------------------------------------------

void rtf_task_start(struct rtf_task *t);

void rtf_task_wait_period(struct rtf_task *t);

uint64_t rtf_task_get_accepted_runtime(struct rtf_task *t);

#endif // RETIF_LIB_H
