#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "logger.h"
#include "retif_daemon.h"
#include "retif_utils.h"

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
static struct rtf_reply req_connection(struct rtf_daemon* data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    INFO("Received CONNECTION REQ from pid: %d\n", req.payload.ids.pid);

    rtf_carrier_set_pid(&(data->chann), cli_id, req.payload.ids.pid);
    rep.rep_type = RTF_CONNECTION_OK;

    INFO("%d connected with success. Assigned id: %d\n", req.payload.ids.pid, cli_id);

    return rep;
}

/**
 * @internal
 *
 * Client wants to create a reservation
 *
 * @endinternal
 */
static struct rtf_reply req_task_create(struct rtf_daemon* data, int cli_id)
{
    int rtf_id, res, pid;
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_CREATE REQ from client: %d\n", cli_id);

    pid = data->chann.client[cli_id].pid;
    res = rtf_scheduler_task_create(&(data->sched), &req.payload.param, pid);
    rtf_id = data->sched.last_task_id;

    if(res == RTF_NO)
    {
        rep.rep_type = RTF_TASK_CREATE_ERR;
        rep.payload = -1;
        LOG("It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RTF_PARTIAL)
    {
        rep.rep_type = RTF_TASK_CREATE_PART;
        rep.payload = rtf_id;
        LOG("Task created with min budget. Res. id: %d\n", rtf_id);
    }
    else
    {
        rep.rep_type = RTF_TASK_CREATE_OK;
        rep.payload = rtf_id;
        LOG("It is possible to guarantee these parameters. Res. id: %d\n", rtf_id);
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
static struct rtf_reply req_task_modify(struct rtf_daemon* data, int cli_id)
{
    int res;
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_MODIFY REQ for rsv: %d\n", req.payload.ids.rsvid);

    res = rtf_scheduler_task_change(&(data->sched), &req.payload.param, req.payload.ids.rsvid);
    res = req.payload.ids.rsvid; // TO BE DELETED

    if(res == RTF_NO)
    {
        rep.rep_type = RTF_TASK_MODIFY_ERR;
        LOG("It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RTF_PARTIAL)
    {
        rep.rep_type = RTF_TASK_MODIFY_PART;
        LOG("Reservation modified with min budget.\n");
    }
    else
    {
        rep.rep_type = RTF_TASK_MODIFY_OK;
        LOG("It is possible to guarantee these parameters.\n");
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
static struct rtf_reply req_task_attach(struct rtf_daemon* data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_ATTACH REQ for res: %d. PID: %d will be attached.\n",
            req.payload.ids.rsvid, req.payload.ids.pid);

    if(rtf_scheduler_task_attach(&(data->sched), req.payload.ids.rsvid, req.payload.ids.pid) < 0)
        rep.rep_type = RTF_TASK_ATTACH_ERR;
    else
        rep.rep_type = RTF_TASK_ATTACH_OK;

    return rep;
}


/**
 * @internal
 *
 * Client wants to detach the flow of execution from the reservation
 *
 * @endinternal
 */
static struct rtf_reply req_task_detach(struct rtf_daemon* data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_DETACH REQ for res: %d. The thread will be detached\n",
        req.payload.ids.rsvid);

    if(rtf_scheduler_task_detach(&(data->sched), req.payload.ids.rsvid) < 0)
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
static struct rtf_reply req_task_destroy(struct rtf_daemon* data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_DESTROY REQ for res: %d. The thread will be detached\n",
        req.payload.ids.rsvid);

    if(rtf_scheduler_task_destroy(&(data->sched), req.payload.ids.rsvid) < 0)
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
int rtf_daemon_check_for_fail(struct rtf_daemon* data, int cli_id)
{
    enum CLIENT_STATE st;
    pid_t pid;

    st = rtf_carrier_get_state(&(data->chann), cli_id);

    if(st != ERROR && st != DISCONNECTED)
        return 0;

    INFO("Client %d disconnected. Its reservation will be destroyed.\n", cli_id);

    rtf_carrier_set_state(&(data->chann), cli_id, EMPTY);
    pid = rtf_carrier_get_pid(&(data->chann), cli_id);

    rtf_scheduler_delete(&(data->sched), pid);

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
int rtf_daemon_check_for_update(struct rtf_daemon* data, int cli_id)
{
    int is_updated;
    enum CLIENT_STATE st;

    is_updated = rtf_carrier_is_updated(&(data->chann), cli_id);
    st = rtf_carrier_get_state(&(data->chann), cli_id);

    if(st == CONNECTED && is_updated)
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
int rtf_daemon_process_req(struct rtf_daemon* data, int cli_id)
{
    struct rtf_reply rep;
    struct rtf_request req;

    req = rtf_carrier_get_req(&(data->chann), cli_id);

    switch(req.req_type)
    {
        case RTF_CONNECTION:
            rep = req_connection(data, cli_id);
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
void rtf_daemon_handle_req(struct rtf_daemon* data, int cli_id)
{
    int sent;

    if(rtf_daemon_check_for_fail(data, cli_id))
        return;

    if(!rtf_daemon_check_for_update(data, cli_id))
        return;

    sent = rtf_daemon_process_req(data, cli_id);
    rtf_carrier_req_clear(&(data->chann), cli_id);

    if(sent <= 0)
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
int rtf_daemon_init(struct rtf_daemon* data)
{
    if(rtf_carrier_init(&(data->chann)) < 0)
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
void rtf_daemon_loop(struct rtf_daemon* data)
{
    while(1)
    {
        rtf_carrier_update(&(data->chann));

        for(int i = 0; i <= rtf_carrier_get_conn(&(data->chann)); i++)
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
void rtf_daemon_dump(struct rtf_daemon* data)
{
    LOG("###### SYS INFO ######\n");
    rtf_scheduler_dump(&(data->sched));
    LOG("######################\n");
    LOG("###### CONNECTION INFO ######\n");
    rtf_carrier_dump(&(data->chann));
    LOG("#############################\n");
}

/**
 * @internal
 *
 * Stops all services of daemon safely, freeing memory,
 * restoring kernel params and stopping loop.
 *
 * @endinternal
 */
void rtf_daemon_destroy(struct rtf_daemon* data)
{
    struct rtf_task* t;

    while(1)
    {
        t = rtf_taskset_remove_top(&(data->tasks));

        if(t == NULL)
            break;

        rtf_task_destroy(t);
    }

    rtf_scheduler_destroy(&(data->sched));
}