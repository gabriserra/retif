#ifndef RETIF_TYPES_H
#define RETIF_TYPES_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>

#ifndef RETIF_PLUGIN_H
#    define PLUGIN_MAX_NAME 32
#    define PLUGIN_MAX_PATH 1024
#endif

#ifndef RETIF_CHANNEL_H
#    define CHANNEL_MAX_SIZE SET_MAX_SIZE
#endif

typedef uint32_t rtf_id_t;
typedef uint32_t plgid_t;

enum REQ_TYPE
{
    RTF_CONNECTION,
    RTF_CONNECTIONS_INFO,
    RTF_CONNECTION_INFO,
    RTF_PLUGINS_INFO,
    RTF_PLUGIN_INFO,
    RTF_PLUGIN_CPU_INFO,
    RTF_TASKS_INFO,
    RTF_TASK_INFO,
    RTF_TASK_MONITOR,
    RTF_TASK_CREATE,
    RTF_TASK_MODIFY,
    RTF_TASK_ATTACH,
    RTF_TASK_DETACH,
    RTF_TASK_DESTROY,
    RTF_DECONNECTION
};

enum REP_TYPE
{
    RTF_REQUEST_ERR,
    RTF_CONNECTION_OK,
    RTF_CONNECTION_ERR,
    RTF_CONNECTIONS_INFO_OK,
    RTF_CONNECTION_INFO_OK,
    RTF_CONNECTION_INFO_ERR,
    RTF_PLUGINS_INFO_OK,
    RTF_PLUGIN_INFO_OK,
    RTF_PLUGIN_INFO_ERR,
    RTF_PLUGIN_CPU_INFO_OK,
    RTF_PLUGIN_CPU_INFO_ERR,
    RTF_TASKS_INFO_OK,
    RTF_TASK_INFO_OK,
    RTF_TASK_INFO_ERR,
    RTF_TASK_CREATE_OK,
    RTF_TASK_CREATE_PART,
    RTF_TASK_CREATE_ERR,
    RTF_TASK_MODIFY_OK,
    RTF_TASK_MODIFY_PART,
    RTF_TASK_MODIFY_ERR,
    RTF_TASK_ATTACH_OK,
    RTF_TASK_ATTACH_ERR,
    RTF_TASK_DETACH_OK,
    RTF_TASK_DETACH_ERR,
    RTF_TASK_DESTROY_OK,
    RTF_TASK_DESTROY_ERR,
    RTF_DECONNECTION_OK,
    RTF_DECONNECTION_ERR
};

#ifndef RETIF_PUBLIC_TYPES
#    define RETIF_PUBLIC_TYPES

#    define RTF_OK 1
#    define RTF_FAIL 0
#    define RTF_ERROR -1
#    define RTF_PARTIAL 0
#    define RTF_NO -1

enum CLIENT_STATE
{
    EMPTY,
    CONNECTED,
    DISCONNECTED,
    ERROR
};

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
    int cputot;
};
struct rtf_cpu_info
{
    int cpunum;
    float freeu;
    int ntask;
};

#endif

struct rtf_ids
{
    pid_t pid;
    rtf_id_t rsvid;
};

struct rtf_desc
{
    pid_t pid;
    rtf_id_t rsvid;
};

struct rtf_request
{
    enum REQ_TYPE req_type;
    union
    {
        struct
        {
            int id;
            int desc;
        } q;
        struct rtf_ids ids;
        struct rtf_params param;
    } payload;
};

struct rtf_reply
{
    enum REP_TYPE rep_type;
    union
    {
        float response;
        int nconnected;
        int ntask;
        int nplugin;
        struct rtf_client_info client;
        struct rtf_task_info task;
        struct rtf_plugin_info plugin;
        struct rtf_cpu_info cpu;
    } payload;
};

struct rtf_client
{
    enum CLIENT_STATE state;
    pid_t pid;
};

#endif // RETIF_TYPES_H
