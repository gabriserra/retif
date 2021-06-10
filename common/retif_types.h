#ifndef RETIF_TYPES_H
#define RETIF_TYPES_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#define RETIF_OK                      1
#define RETIF_FAIL                    0
#define RETIF_ERROR                   -1
#define RETIF_PARTIAL                 0
#define RETIF_NO                      -1

#ifndef RETIF_PLUGIN_H
    #define PLUGIN_MAX_NAME         32
    #define PLUGIN_MAX_PATH         1024
#endif

typedef uint32_t retif_id_t;
typedef uint32_t plgid_t;

enum REQ_TYPE
{
    RETIF_CONNECTION,
    RETIF_TASK_CREATE,
    RETIF_TASK_MODIFY,
    RETIF_TASK_ATTACH,
    RETIF_TASK_DETACH,
    RETIF_TASK_DESTROY,
    RETIF_DECONNECTION
};

enum REP_TYPE
{
    RETIF_REQUEST_ERR,
    RETIF_CONNECTION_OK,
    RETIF_CONNECTION_ERR,
    RETIF_TASK_CREATE_OK,
    RETIF_TASK_CREATE_PART,
    RETIF_TASK_CREATE_ERR,
    RETIF_TASK_MODIFY_OK,
    RETIF_TASK_MODIFY_PART,
    RETIF_TASK_MODIFY_ERR,
    RETIF_TASK_ATTACH_OK,
    RETIF_TASK_ATTACH_ERR,
    RETIF_TASK_DETACH_OK,
    RETIF_TASK_DETACH_ERR,
    RETIF_TASK_DESTROY_OK,
    RETIF_TASK_DESTROY_ERR,
    RETIF_DECONNECTION_OK,
    RETIF_DECONNECTION_ERR
};

enum CLIENT_STATE
{
    EMPTY,
    CONNECTED,
    DISCONNECTED,
    ERROR
};

#ifndef RETIF_PUBLIC_TYPES
#define RETIF_PUBLIC_TYPES

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

#endif	// RETIF_TYPES_H
