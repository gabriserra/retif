#ifndef retif_TYPES_H
#define retif_TYPES_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#define retif_OK                      1
#define retif_FAIL                    0
#define retif_ERROR                   -1
#define retif_PARTIAL                 0
#define retif_NO                      -1

#ifndef retif_PLUGIN_H
    #define PLUGIN_MAX_NAME         10
    #define PLUGIN_MAX_PATH         40
#endif

typedef uint32_t retif_id_t;
typedef uint32_t plgid_t;

enum REQ_TYPE
{
    retif_CONNECTION,
    retif_TASK_CREATE,
    retif_TASK_MODIFY,
    retif_TASK_ATTACH,
    retif_TASK_DETACH,
    retif_TASK_DESTROY,
    retif_DECONNECTION
};

enum REP_TYPE
{
    retif_REQUEST_ERR,
    retif_CONNECTION_OK,
    retif_CONNECTION_ERR,
    retif_TASK_CREATE_OK,
    retif_TASK_CREATE_PART,
    retif_TASK_CREATE_ERR,
    retif_TASK_MODIFY_OK,
    retif_TASK_MODIFY_PART,
    retif_TASK_MODIFY_ERR,
    retif_TASK_ATTACH_OK,
    retif_TASK_ATTACH_ERR,
    retif_TASK_DETACH_OK,
    retif_TASK_DETACH_ERR,
    retif_TASK_DESTROY_OK,
    retif_TASK_DESTROY_ERR,
    retif_DECONNECTION_OK,
    retif_DECONNECTION_ERR
};

enum CLIENT_STATE
{
    EMPTY,
    CONNECTED,
    DISCONNECTED,
    ERROR
};

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

#endif

struct retif_ids
{
    pid_t pid;
    retif_id_t rsvid;
};

struct retif_request
{
    enum REQ_TYPE req_type;
    union
    {
        struct retif_ids ids;
        struct retif_params param;
    } payload;
};

struct retif_reply
{
    enum REP_TYPE rep_type;
    float payload;
};

struct retif_client
{
    enum CLIENT_STATE state;
    pid_t pid;
};

#endif	// retif_TYPES_H
