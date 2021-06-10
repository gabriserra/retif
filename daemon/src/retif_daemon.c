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
static struct retif_reply req_connection(struct retif_daemon* data, int cli_id)
{
    struct retif_reply rep;
    struct retif_request req;

    req = retif_carrier_get_req(&(data->chann), cli_id);

    INFO("Received CONNECTION REQ from pid: %d\n", req.payload.ids.pid);

    retif_carrier_set_pid(&(data->chann), cli_id, req.payload.ids.pid);
    rep.rep_type = RETIF_CONNECTION_OK;

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
static struct retif_reply req_task_create(struct retif_daemon* data, int cli_id)
{
    int retif_id, res, pid;
    struct retif_reply rep;
    struct retif_request req;

    req = retif_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_CREATE REQ from client: %d\n", cli_id);

    pid = data->chann.client[cli_id].pid;
    res = retif_scheduler_task_create(&(data->sched), &req.payload.param, pid);
    retif_id = data->sched.last_task_id;

    if(res == RETIF_NO)
    {
        rep.rep_type = RETIF_TASK_CREATE_ERR;
        rep.payload = -1;
        LOG("It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RETIF_PARTIAL)
    {
        rep.rep_type = RETIF_TASK_CREATE_PART;
        rep.payload = retif_id;
        LOG("Task created with min budget. Res. id: %d\n", retif_id);
    }
    else
    {
        rep.rep_type = RETIF_TASK_CREATE_OK;
        rep.payload = retif_id;
        LOG("It is possible to guarantee these parameters. Res. id: %d\n", retif_id);
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
static struct retif_reply req_task_modify(struct retif_daemon* data, int cli_id)
{
    int res;
    struct retif_reply rep;
    struct retif_request req;

    req = retif_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_MODIFY REQ for rsv: %d\n", req.payload.ids.rsvid);

    res = retif_scheduler_task_change(&(data->sched), &req.payload.param, req.payload.ids.rsvid);
    res = req.payload.ids.rsvid; // TO BE DELETED

    if(res == RETIF_NO)
    {
        rep.rep_type = RETIF_TASK_MODIFY_ERR;
        LOG("It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RETIF_PARTIAL)
    {
        rep.rep_type = RETIF_TASK_MODIFY_PART;
        LOG("Reservation modified with min budget.\n");
    }
    else
    {
        rep.rep_type = RETIF_TASK_MODIFY_OK;
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
static struct retif_reply req_task_attach(struct retif_daemon* data, int cli_id)
{
    struct retif_reply rep;
    struct retif_request req;

    req = retif_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_ATTACH REQ for res: %d. PID: %d will be attached.\n",
            req.payload.ids.rsvid, req.payload.ids.pid);

    if(retif_scheduler_task_attach(&(data->sched), req.payload.ids.rsvid, req.payload.ids.pid) < 0)
        rep.rep_type = RETIF_TASK_ATTACH_ERR;
    else
        rep.rep_type = RETIF_TASK_ATTACH_OK;

    return rep;
}


/**
 * @internal
 *
 * Client wants to detach the flow of execution from the reservation
 *
 * @endinternal
 */
static struct retif_reply req_task_detach(struct retif_daemon* data, int cli_id)
{
    struct retif_reply rep;
    struct retif_request req;

    req = retif_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_DETACH REQ for res: %d. The thread will be detached\n",
        req.payload.ids.rsvid);

    if(retif_scheduler_task_detach(&(data->sched), req.payload.ids.rsvid) < 0)
        rep.rep_type = RETIF_TASK_DETACH_ERR;
    else
        rep.rep_type = RETIF_TASK_DETACH_OK;

    return rep;
}

/**
 * @internal
 *
 * Client wants to destroy given reservation
 *
 * @endinternal
 */
static struct retif_reply req_task_destroy(struct retif_daemon* data, int cli_id)
{
    struct retif_reply rep;
    struct retif_request req;

    req = retif_carrier_get_req(&(data->chann), cli_id);

    LOG("Received RSV_DESTROY REQ for res: %d. The thread will be detached\n",
        req.payload.ids.rsvid);

    if(retif_scheduler_task_destroy(&(data->sched), req.payload.ids.rsvid) < 0)
        rep.rep_type = RETIF_TASK_DESTROY_ERR;
    else
        rep.rep_type = RETIF_TASK_DESTROY_OK;

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
int retif_daemon_check_for_fail(struct retif_daemon* data, int cli_id)
{
    enum CLIENT_STATE st;
    pid_t pid;

    st = retif_carrier_get_state(&(data->chann), cli_id);

    if(st != ERROR && st != DISCONNECTED)
        return 0;

    INFO("Client %d disconnected. Its reservation will be destroyed.\n", cli_id);

    retif_carrier_set_state(&(data->chann), cli_id, EMPTY);
    pid = retif_carrier_get_pid(&(data->chann), cli_id);

    retif_scheduler_delete(&(data->sched), pid);

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
int retif_daemon_check_for_update(struct retif_daemon* data, int cli_id)
{
    int is_updated;
    enum CLIENT_STATE st;

    is_updated = retif_carrier_is_updated(&(data->chann), cli_id);
    st = retif_carrier_get_state(&(data->chann), cli_id);

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
int retif_daemon_process_req(struct retif_daemon* data, int cli_id)
{
    struct retif_reply rep;
    struct retif_request req;

    req = retif_carrier_get_req(&(data->chann), cli_id);

    switch(req.req_type)
    {
        case RETIF_CONNECTION:
            rep = req_connection(data, cli_id);
            break;
        case RETIF_TASK_CREATE:
            rep = req_task_create(data, cli_id);
            break;
        case RETIF_TASK_MODIFY:
            rep = req_task_modify(data, cli_id);
            break;
        case RETIF_TASK_ATTACH:
            rep = req_task_attach(data, cli_id);
            break;
        case RETIF_TASK_DETACH:
            rep = req_task_detach(data, cli_id);
            break;
        case RETIF_TASK_DESTROY:
            rep = req_task_destroy(data, cli_id);
            break;
        default:
            rep.rep_type = RETIF_REQUEST_ERR;
    }

    return retif_carrier_send(&(data->chann), &rep, cli_id);
}

/**
 * @internal
 *
 * After checking connection status, if client has sent something
 * pass to process request
 *
 * @endinternal
 */
void retif_daemon_handle_req(struct retif_daemon* data, int cli_id)
{
    int sent;

    if(retif_daemon_check_for_fail(data, cli_id))
        return;

    if(!retif_daemon_check_for_update(data, cli_id))
        return;

    sent = retif_daemon_process_req(data, cli_id);
    retif_carrier_req_clear(&(data->chann), cli_id);

    if(sent <= 0)
        retif_carrier_set_state(&(data->chann), cli_id, ERROR);
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
int retif_daemon_init(struct retif_daemon* data)
{
    if(retif_carrier_init(&(data->chann)) < 0)
        return -1;

    retif_taskset_init(&(data->tasks));

    if (retif_scheduler_init(&(data->sched), &(data->tasks)) < 0)
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
void retif_daemon_loop(struct retif_daemon* data)
{
    while(1)
    {
        retif_carrier_update(&(data->chann));

        for(int i = 0; i <= retif_carrier_get_conn(&(data->chann)); i++)
            retif_daemon_handle_req(data, i);
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
void retif_daemon_dump(struct retif_daemon* data)
{
    LOG("###### SYS INFO ######\n");
    retif_scheduler_dump(&(data->sched));
    LOG("######################\n");
    LOG("###### CONNECTION INFO ######\n");
    retif_carrier_dump(&(data->chann));
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
void retif_daemon_destroy(struct retif_daemon* data)
{
    struct retif_task* t;

    while(1)
    {
        t = retif_taskset_remove_top(&(data->tasks));

        if(t == NULL)
            break;

        retif_task_destroy(t);
    }

    retif_scheduler_destroy(&(data->sched));
}
