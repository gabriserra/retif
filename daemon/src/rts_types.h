#ifndef RTS_TYPES_H
#define RTS_TYPES_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#define RTS_OK                      1
#define RTS_FAIL                    0
#define RTS_ERROR                   -1
#define RTS_PARTIAL                 0
#define RTS_NO                      -1

#ifndef RTS_PLUGIN_H
    #define PLUGIN_MAX_NAME         10
    #define PLUGIN_MAX_PATH         40
#endif

typedef uint32_t rts_id_t;

enum REQ_TYPE 
{
    RTS_CONNECTION,
    RTS_TASK_CREATE,
    RTS_TASK_MODIFY,
    RTS_TASK_ATTACH,
    RTS_TASK_DETACH,
    RTS_TASK_DESTROY,
    RTS_DECONNECTION
};

enum REP_TYPE 
{
    RTS_REQUEST_ERR,
    RTS_CONNECTION_OK,
    RTS_CONNECTION_ERR,
    RTS_TASK_CREATE_OK,
    RTS_TASK_CREATE_PART,
    RTS_TASK_CREATE_ERR,
    RTS_TASK_MODIFY_OK,
    RTS_TASK_MODIFY_PART,
    RTS_TASK_MODIFY_ERR,
    RTS_TASK_ATTACH_OK,
    RTS_TASK_ATTACH_ERR,
    RTS_TASK_DETACH_OK,
    RTS_TASK_DETACH_ERR,
    RTS_TASK_DESTROY_OK,
    RTS_TASK_DESTROY_ERR,
    RTS_DECONNECTION_OK,
    RTS_DECONNECTION_ERR
};

enum CLIENT_STATE 
{
    EMPTY,
    CONNECTED,
    DISCONNECTED,
    ERROR
};

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

#endif

struct rts_ids 
{
    pid_t pid;
    rts_id_t rsvid;
};

struct rts_request 
{
    enum REQ_TYPE req_type;
    union 
    {
        struct rts_ids ids;
        struct rts_params param;
    } payload;
};

struct rts_reply 
{
    enum REP_TYPE rep_type;
    float payload;
};

struct rts_client 
{
    enum CLIENT_STATE state;
    pid_t pid;
};

#endif	// RTS_TYPES_H

