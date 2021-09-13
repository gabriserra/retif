#include "retif_daemon.h"
#include "logger.h"
#include "retif_utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// REQUEST HANDLING METHODS
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * Client is requesting to connect to daemon services.
 *
 * @endinternal
 */
static struct rtf_reply req_connection(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(INFO, "Received CONNECTION REQ from pid: %d\n", req.payload.ids.pid);

    rtf_carrier_set_pid(&(data->chann), cli_id, req.payload.ids.pid);
    rep.rep_type = RTF_CONNECTION_OK;

    LOG(INFO, "%d connected with success. Assigned id: %d\n",
        req.payload.ids.pid, cli_id);

    return rep;
}

static struct rtf_reply req_connections_info(struct rtf_daemon *data,
    int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;
    int nclients = 0;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG, "Received RTF_CONNECTIONS_INFO from client: %d\n", cli_id);

    for (int i = 0; i < CHANNEL_MAX_SIZE; i++)
        if (data->chann.client[i].pid != 0)
            nclients++;

    rep.rep_type = RTF_CONNECTIONS_INFO_OK;
    rep.payload.nconnected = nclients;

    return rep;
}

static struct rtf_reply req_plugins_info(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG, "Received RTF_PLUGINGS_INFO from client: %d\n", cli_id);

    rep.rep_type = RTF_PLUGINS_INFO_OK;
    rep.payload.nplugin = data->sched.num_of_plugins;

    return rep;
}

static struct rtf_reply req_tasks_info(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG, "Received RTF_TASKS_INFO from client: %d\n", cli_id);

    rep.rep_type = RTF_PLUGINS_INFO_OK;
    rep.payload.ntask = data->sched.taskset->tasks.n;

    return rep;
}

static struct rtf_reply req_connection_info(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;
    struct rtf_client *client;
    int cdesc;

    req = rtf_carrier_get_req(&(data->chann), cli_id);
    cdesc = req.payload.q.desc;

    LOG(DEBUG, "Received RTF_CONNECTION_INFO from client: %d\n", cli_id);

    if (cdesc > CHANNEL_MAX_SIZE || data->chann.client[cdesc].pid == 0)
    {
        rep.rep_type = RTF_CONNECTION_INFO_ERR;
    }
    else
    {
        rep.rep_type = RTF_CONNECTION_INFO_OK;
        rep.payload.client.pid = data->chann.client[cdesc].pid;
        rep.payload.client.state = data->chann.client[cdesc].state;
    }

    return rep;
}

static struct rtf_reply req_plugin_info(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;
    struct rtf_client *client;
    int pdesc;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    pdesc = req.payload.q.desc;

    LOG(DEBUG, "Received RTF_PLUGIN_INFO from client: %d\n", cli_id);

    if (pdesc >= data->sched.num_of_plugins)
    {
        rep.rep_type = RTF_PLUGIN_INFO_ERR;
    }
    else
    {
        rep.rep_type = RTF_PLUGIN_INFO_OK;
        strncpy(rep.payload.plugin.name, data->sched.plugin[pdesc].name,
            PLUGIN_MAX_NAME);
        rep.payload.plugin.cpunum = data->sched.plugin[pdesc].cpunum;
    }

    return rep;
}

static struct rtf_reply req_plugin_cpu_info(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;
    struct rtf_client *client;
    int pdesc, cpuid;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    pdesc = req.payload.q.desc;
    cpuid = req.payload.q.id;

    LOG(DEBUG, "Received RTF_PLUGIN_CPU_INFO from client: %d\n", cli_id);

    if (pdesc >= data->sched.num_of_plugins ||
        cpuid >= data->sched.plugin[pdesc].cputot)
    {
        rep.rep_type = RTF_PLUGIN_CPU_INFO_ERR;
    }
    else if (cpuid >= data->sched.plugin[pdesc].cputot)
    {
        rep.rep_type = RTF_PLUGIN_CPU_INFO_OK;
        rep.payload.cpu.cpunum = data->sched.plugin[pdesc].cpulist[cpuid];
        rep.payload.cpu.ntask =
            data->sched.plugin[pdesc].task_count_percpu[cpuid];
        rep.payload.cpu.freeu =
            data->sched.plugin[pdesc].util_free_percpu[cpuid];
    }

    return rep;
}

static struct rtf_reply req_task_info(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;
    struct rtf_client *client;
    struct rtf_task *task;

    req = rtf_carrier_get_req(&(data->chann), cli_id);
    task = rtf_taskset_search(&(data->tasks), req.payload.q.desc);

    LOG(DEBUG, "Received RTF_TASK_INFO from client: %d\n", cli_id);

    if (task == NULL)
    {
        rep.rep_type = RTF_TASK_INFO_ERR;
    }
    else
    {
        rep.rep_type = RTF_TASK_INFO_OK;
        rep.payload.task.tid = task->tid;
        rep.payload.task.ppid = task->ptid;
        rep.payload.task.priority = task->schedprio;
        rep.payload.task.period = task->params.period;
        rep.payload.task.pluginid = task->pluginid;
    }

    return rep;
}

/**
 * @internal
 *
 * Client wants to create a reservation
 *
 * @endinternal
 */
static struct rtf_reply req_task_create(struct rtf_daemon *data, int cli_id)
{
    int rtf_id, res, pid;
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG, "Received RSV_CREATE REQ from client: %d\n", cli_id);

    pid = data->chann.client[cli_id].pid;
    res = rtf_scheduler_task_create(&(data->sched), &req.payload.param, pid);
    rtf_id = data->sched.last_task_id;

    if (res == RTF_NO)
    {
        rep.rep_type = RTF_TASK_CREATE_ERR;
        rep.payload.response = -1;
        LOG(DEBUG, "It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RTF_PARTIAL)
    {
        rep.rep_type = RTF_TASK_CREATE_PART;
        rep.payload.response = rtf_id;
        LOG(DEBUG, "Task created with min budget. Res. id: %d\n", rtf_id);
    }
    else
    {
        rep.rep_type = RTF_TASK_CREATE_OK;
        rep.payload.response = rtf_id;
        LOG(DEBUG,
            "It is possible to guarantee these parameters. Res. id: %d\n",
            rtf_id);
    }

    return rep;
}

/**
 * @internal
 *
 * Client wants to modify a reservation
 *
 * @endinternal
 */
static struct rtf_reply req_task_modify(struct rtf_daemon *data, int cli_id)
{
    int res;
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG, "Received RSV_MODIFY REQ for rsv: %d\n", req.payload.ids.rsvid);

    res = rtf_scheduler_task_change(&(data->sched), &req.payload.param,
        req.payload.ids.rsvid);

    if (res == RTF_NO)
    {
        rep.rep_type = RTF_TASK_MODIFY_ERR;
        LOG(DEBUG, "It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RTF_PARTIAL)
    {
        rep.rep_type = RTF_TASK_MODIFY_PART;
        LOG(DEBUG, "Reservation modified with min budget.\n");
    }
    else
    {
        rep.rep_type = RTF_TASK_MODIFY_OK;
        LOG(DEBUG, "It is possible to guarantee these parameters.\n");
    }

    return rep;
}

/**
 * @internal
 *
 * Client wants to attach a flow of execution to the reservation
 *
 * @endinternal
 */
static struct rtf_reply req_task_attach(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG,
        "Received RSV_ATTACH REQ for res: %d. PID: %d will be attached.\n",
        req.payload.ids.rsvid, req.payload.ids.pid);

    if (rtf_scheduler_task_attach(&(data->sched), req.payload.ids.rsvid,
            req.payload.ids.pid) < 0)
    {
        rep.rep_type = RTF_TASK_ATTACH_ERR;
        LOG(WARNING, "Unable to attach PID: %d to given task.\n",
            req.payload.ids.pid);
    }
    else
    {
        rep.rep_type = RTF_TASK_ATTACH_OK;
        LOG(INFO, "PID: %d attached.\n", req.payload.ids.pid);
    }

    return rep;
}

/**
 * @internal
 *
 * Client wants to detach the flow of execution from the reservation
 *
 * @endinternal
 */
static struct rtf_reply req_task_detach(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG,
        "Received RSV_DETACH REQ for res: %d. The thread will be detached\n",
        req.payload.ids.rsvid);

    if (rtf_scheduler_task_detach(&(data->sched), req.payload.ids.rsvid) < 0)
        rep.rep_type = RTF_TASK_DETACH_ERR;
    else
        rep.rep_type = RTF_TASK_DETACH_OK;

    return rep;
}

/**
 * @internal
 *
 * Client wants to destroy given reservation
 *
 * @endinternal
 */
static struct rtf_reply req_task_destroy(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG(DEBUG,
        "Received RSV_DESTROY REQ for res: %d. The thread will be detached\n",
        req.payload.ids.rsvid);

    if (rtf_scheduler_task_destroy(&(data->sched), req.payload.ids.rsvid) < 0)
        rep.rep_type = RTF_TASK_DESTROY_ERR;
    else
        rep.rep_type = RTF_TASK_DESTROY_OK;

    return rep;
}

// -----------------------------------------------------------------------------
// PRIVATE HELPER METHODS
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * Checks if the client with id @p cli_id is DISCONNECTED or an error
 * was occurred during communication. In those cases, set the client
 * descriptor as empty and deletes all its reservation. Returns 0 if
 * client is still connected, 1 if client it is in a fail-state
 *
 * @endinternal
 */
int rtf_daemon_check_for_fail(struct rtf_daemon *data, int cli_id)
{
    enum CLIENT_STATE st;
    pid_t pid;

    st = rtf_carrier_get_state(&(data->chann), cli_id);

    if (st != ERROR && st != DISCONNECTED)
        return 0;

    LOG(INFO, "Client %d disconnected. Its reservation will be destroyed.\n",
        cli_id);

    rtf_carrier_set_state(&(data->chann), cli_id, EMPTY);
    pid = rtf_carrier_get_pid(&(data->chann), cli_id);

    rtf_scheduler_delete(&(data->sched), pid);
    rtf_carrier_set_pid(&(data->chann), cli_id, 0);

    return 1;
}

/**
 * @internal
 *
 * Checks if the client with id @p cli_id has been sent request during the
 * last daemon loop.
 *
 * @endinternal
 */
int rtf_daemon_check_for_update(struct rtf_daemon *data, int cli_id)
{
    int is_updated;
    enum CLIENT_STATE st;

    is_updated = rtf_carrier_is_updated(&(data->chann), cli_id);
    st = rtf_carrier_get_state(&(data->chann), cli_id);

    if (st == CONNECTED && is_updated)
        return 1;

    return 0;
}

/**
 * @internal
 *
 * Receive a request, process it and sent out the reply
 *
 * @endinternal
 */
int rtf_daemon_process_req(struct rtf_daemon *data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    switch (req.req_type)
    {
    case RTF_CONNECTION:
        rep = req_connection(data, cli_id);
        break;
    case RTF_CONNECTIONS_INFO:
        rep = req_connections_info(data, cli_id);
        break;
    case RTF_CONNECTION_INFO:
        rep = req_connection_info(data, cli_id);
        break;
    case RTF_PLUGINS_INFO:
        rep = req_plugins_info(data, cli_id);
        break;
    case RTF_PLUGIN_INFO:
        rep = req_plugin_info(data, cli_id);
        break;
    case RTF_PLUGIN_CPU_INFO:
        rep = req_plugin_cpu_info(data, cli_id);
        break;
    case RTF_TASKS_INFO:
        rep = req_tasks_info(data, cli_id);
        break;
    case RTF_TASK_INFO:
        rep = req_task_info(data, cli_id);
        break;
    case RTF_TASK_CREATE:
        rep = req_task_create(data, cli_id);
        break;
    case RTF_TASK_MODIFY:
        rep = req_task_modify(data, cli_id);
        break;
    case RTF_TASK_ATTACH:
        rep = req_task_attach(data, cli_id);
        break;
    case RTF_TASK_DETACH:
        rep = req_task_detach(data, cli_id);
        break;
    case RTF_TASK_DESTROY:
        rep = req_task_destroy(data, cli_id);
        break;
    default:
        rep.rep_type = RTF_REQUEST_ERR;
    }

    return rtf_carrier_send(&(data->chann), &rep, cli_id);
}

/**
 * @internal
 *
 * After checking connection status, if client has sent something
 * pass to process request
 *
 * @endinternal
 */
void rtf_daemon_handle_req(struct rtf_daemon *data, int cli_id)
{
    int sent;

    if (rtf_daemon_check_for_fail(data, cli_id))
        return;

    if (!rtf_daemon_check_for_update(data, cli_id))
        return;

    sent = rtf_daemon_process_req(data, cli_id);
    rtf_carrier_req_clear(&(data->chann), cli_id);

    if (sent <= 0)
        rtf_carrier_set_state(&(data->chann), cli_id, ERROR);
}

// -----------------------------------------------------------------------------
// PUBLIC METHODS
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * Initializes daemon applying custom configuration and initializing carrier,
 * taskset & scheduler.
 *
 * @endinternal
 */
int rtf_daemon_init(struct rtf_daemon *data)
{
    if (rtf_carrier_init(&(data->chann)) < 0)
        return -1;

    rtf_taskset_init(&(data->tasks));

    if (rtf_scheduler_init(&(data->sched), &(data->tasks)) < 0)
        return -1;

    return 0;
}

/**
 * @internal
 *
 * Realizes daemon loop, waiting for requests and handling it
 *
 * @endinternal
 */
void rtf_daemon_loop(struct rtf_daemon *data)
{
    while (1)
    {
        rtf_carrier_update(&(data->chann));

        for (int i = 0; i <= rtf_carrier_get_conn(&(data->chann)); i++)
            rtf_daemon_handle_req(data, i);
    }
}

/**
 * @internal
 *
 * Dumps all daemon data info such as connected proccesses,
 * tasks, scheduling plugins and so on.
 *
 * @endinternal
 */
void rtf_daemon_dump(struct rtf_daemon *data)
{
    LOG(DEBUG, "###### SYS INFO ######\n");
    rtf_scheduler_dump(&(data->sched));
    LOG(DEBUG, "######################\n");
    LOG(DEBUG, "###### CONNECTION INFO ######\n");
    rtf_carrier_dump(&(data->chann));
    LOG(DEBUG, "#############################\n");
}

/**
 * @internal
 *
 * Stops all services of daemon safely, freeing memory,
 * restoring kernel params and stopping loop.
 *
 * @endinternal
 */
void rtf_daemon_destroy(struct rtf_daemon *data)
{
    struct rtf_task *t;

    while (1)
    {
        t = rtf_taskset_remove_top(&(data->tasks));

        if (t == NULL)
            break;

        rtf_task_release(t);
    }

    rtf_scheduler_destroy(&(data->sched));
}
