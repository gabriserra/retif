
#include "retif.h"
#include "retif_channel.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// -----------------------------------------------------------------------------
// DEFINITION OF CLOCK CONSTANT
// -----------------------------------------------------------------------------

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
#ifndef CLOCK_THREAD_CPUTIME_ID
#define CLOCK_THREAD_CPUTIME_ID 3
#endif
#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME 0x01
#endif

// -----------------------------------------------------------------------------
// THREAD STUFF
// -----------------------------------------------------------------------------

static struct rtf_access main_channel;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct LOGGER *logger;

static int rtf_task_communicate(struct rtf_access *c)
{
    int ret = RTF_ERROR;

    pthread_mutex_lock(&mutex);

    if (rtf_access_send(c) != RTF_ERROR)
        ret = rtf_access_recv(c);

    pthread_mutex_unlock(&mutex);

    if (ret == RTF_ERROR)
        return RTF_ERROR;

    return RTF_OK;
}

// -----------------------------------------------------------------------------
// TIME UTILS
// -----------------------------------------------------------------------------

static void time_add_ms(struct timespec *t, uint32_t ms)
{
    t->tv_sec += MILLI_TO_SEC(ms);
    t->tv_nsec += MILLI_TO_NANO(ms % EXP3);

    if (t->tv_nsec > EXP9)
    {
        t->tv_nsec -= EXP9;
        t->tv_sec += 1;
    }
}

static int time_cmp(struct timespec *t1, struct timespec *t2)
{
    if (t1->tv_sec > t2->tv_sec)
        return 1;
    if (t1->tv_sec < t2->tv_sec)
        return -1;

    if (t1->tv_nsec > t2->tv_nsec)
        return 1;
    if (t1->tv_nsec < t2->tv_nsec)
        return -1;

    return 0;
}

static void time_copy(struct timespec *td, struct timespec *ts)
{
    td->tv_sec = ts->tv_sec;
    td->tv_nsec = ts->tv_nsec;
}

static uint32_t timespec_to_ms(struct timespec *t)
{
    uint32_t ms;

    ms = SEC_TO_MILLI(t->tv_sec);
    ms += NANO_TO_MILLI(t->tv_nsec);

    return ms;
}

struct timespec get_thread_time()
{
    struct timespec ts;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return ts;
}

uint32_t get_thread_time_ms()
{
    struct timespec ts;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return timespec_to_ms(&ts);
}

void compute_for(uint32_t exec_milli_max)
{
    uint32_t exec_milli;
    struct timespec t_curr;
    struct timespec t_end;

    exec_milli = rand() % exec_milli_max;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_end);
    time_add_ms(&t_end, exec_milli);

    while (1)
    {
        __asm__("nop"); // simulate computation of something..
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_curr);

        if (time_cmp(&t_curr, &t_end) > 0)
            break;
    }
}

// -----------------------------------------------------------------------------
// GETTER / SETTER
// -----------------------------------------------------------------------------

void rtf_params_init(struct rtf_params *p)
{
    memset(p, 0, sizeof(struct rtf_params));
}

void rtf_params_set_runtime(struct rtf_params *p, uint64_t runtime)
{
    p->runtime = runtime;
}

uint64_t rtf_params_get_runtime(struct rtf_params *p)
{
    return p->runtime;
}

void rtf_params_set_des_runtime(struct rtf_params *p, uint64_t runtime)
{
    p->runtime = runtime;
}

uint64_t rtf_params_get_des_runtime(struct rtf_params *p)
{
    return p->runtime;
}

void rtf_params_set_period(struct rtf_params *p, uint64_t period)
{
    p->period = period;
}

uint64_t rtf_params_get_period(struct rtf_params *p)
{
    return p->period;
}

void rtf_params_set_deadline(struct rtf_params *p, uint64_t deadline)
{
    p->deadline = deadline;
}

uint64_t rtf_params_get_deadline(struct rtf_params *p)
{
    return p->deadline;
}

void rtf_params_set_priority(struct rtf_params *p, uint32_t priority)
{
    p->priority = priority;
}

uint32_t rtf_params_get_priority(struct rtf_params *p)
{
    return p->priority;
}

void rtf_params_set_scheduler(struct rtf_params *p,
    char sched_plugin[PLUGIN_MAX_NAME])
{
    strncpy(p->sched_plugin, sched_plugin, PLUGIN_MAX_NAME);
}

void rtf_params_get_scheduler(struct rtf_params *p,
    char sched_plugin[PLUGIN_MAX_NAME])
{
    strncpy(sched_plugin, p->sched_plugin, PLUGIN_MAX_NAME);
}

void rtf_params_ignore_admission(struct rtf_params *p, uint8_t ignore_admission)
{
    p->ignore_admission = ignore_admission;
}

// -----------------------------------------------------------------------------
// COMMUNICATION WITH DAEMON
// -----------------------------------------------------------------------------

int rtf_connect()
{
    if (rtf_access_init(&main_channel) < 0)
        return RTF_ERROR;

    if (rtf_access_connect(&main_channel) < 0)
        return RTF_ERROR;

    main_channel.req.req_type = RTF_CONNECTION;
    main_channel.req.payload.ids.pid = getpid();

    if (rtf_access_send(&main_channel) < 0)
        return RTF_ERROR;
    if (rtf_access_recv(&main_channel) < 0)
        return RTF_ERROR;

    if (main_channel.rep.rep_type == RTF_CONNECTION_ERR)
        return RTF_FAIL;

    return RTF_OK;
}

void rtf_task_init(struct rtf_task *t)
{
    t->c = &main_channel;
    t->task_id = 0;
}

int rtf_task_create(struct rtf_task *t, struct rtf_params *p)
{
    t->c->req.req_type = RTF_TASK_CREATE;
    memcpy(&(t->p), p, sizeof(struct rtf_params));
    memcpy(&(t->c->req.payload.param), p, sizeof(struct rtf_params));

    if (rtf_task_communicate(t->c) < 0)
        return RTF_ERROR;

    if (t->c->rep.rep_type == RTF_TASK_CREATE_ERR)
        return RTF_FAIL;

    t->task_id = (uint32_t) t->c->rep.payload;
    return RTF_OK;
}

int rtf_task_change(struct rtf_task *t, struct rtf_params *p)
{
    t->c->req.req_type = RTF_TASK_MODIFY;
    memcpy(&(t->p), p, sizeof(struct rtf_params));
    memcpy(&(t->c->req.payload.param), p, sizeof(struct rtf_params));

    if (rtf_task_communicate(t->c) < 0)
        return RTF_ERROR;

    if (t->c->rep.rep_type == RTF_TASK_MODIFY_ERR)
        return RTF_FAIL;

    return RTF_OK;
}

int rtf_task_attach(struct rtf_task *t, pid_t pid)
{
    t->c->req.req_type = RTF_TASK_ATTACH;
    t->c->req.payload.ids.rsvid = t->task_id;
    t->c->req.payload.ids.pid = pid;

    if (rtf_task_communicate(t->c) < 0)
        return RTF_ERROR;

    if (t->c->rep.rep_type == RTF_TASK_ATTACH_ERR)
        return RTF_FAIL;

    return RTF_OK;
}

int rtf_task_detach(struct rtf_task *t)
{
    t->c->req.req_type = RTF_TASK_DETACH;
    t->c->req.payload.ids.rsvid = t->task_id;

    if (rtf_task_communicate(t->c) < 0)
        return RTF_ERROR;

    if (t->c->rep.rep_type == RTF_TASK_DETACH_ERR)
        return RTF_FAIL;

    return RTF_OK;
}

int rtf_task_release(struct rtf_task *t)
{
    t->c->req.req_type = RTF_TASK_DESTROY;
    t->c->req.payload.ids.rsvid = t->task_id;

    if (rtf_task_communicate(t->c) < 0)
        return RTF_ERROR;

    if (t->c->rep.rep_type == RTF_TASK_DESTROY_ERR)
        return RTF_FAIL;

    return RTF_OK;
}

// -----------------------------------------------------------------------------
// LOCAL COMMUNICATIONS
// -----------------------------------------------------------------------------

void rtf_task_start(struct rtf_task *t)
{
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);
    time_copy(&(t->at), &time);
    time_copy(&(t->dl), &time);

    // adds period and deadline
    time_add_ms(&(t->at), t->p.period);
    time_add_ms(&(t->dl), t->p.deadline);
}

void rtf_task_wait_period(struct rtf_task *t)
{
    time_add_ms(&(t->at), t->p.period);
    time_add_ms(&(t->dl), t->p.deadline);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(t->at), NULL);
}

uint64_t rtf_task_get_accepted_budget(struct rtf_task *t)
{
    return t->acc_runtime;
}
