#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "logger.h"
#include "rts_daemon.h"
#include "rts_utils.h"

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
static struct rts_reply req_connection(struct rts_daemon* data, int cli_id)
{
    struct rts_reply rep;
    struct rts_request req;
    
    req = rts_carrier_get_req(&(data->chann), cli_id);
    
    INFO("Received CONNECTION REQ from pid: %d\n", req.payload.ids.pid);

    rts_carrier_set_pid(&(data->chann), cli_id, req.payload.ids.pid);
    rep.rep_type = RTS_CONNECTION_OK;

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
static struct rts_reply req_task_create(struct rts_daemon* data, int cli_id) 
{
    int rts_id, res, pid;
    struct rts_reply rep;
    struct rts_request req;

    req = rts_carrier_get_req(&(data->chann), cli_id);
    
    LOG("Received RSV_CREATE REQ from client: %d\n", cli_id);

    pid = data->chann.client[cli_id].pid;
    res = rts_scheduler_task_create(&(data->sched), &req.payload.param, pid);
    rts_id = data->sched.last_task_id;

    if(res == RTS_NO) 
    {
        rep.rep_type = RTS_TASK_CREATE_ERR;
        rep.payload = -1;
        LOG("It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RTS_PARTIAL)
    {
        rep.rep_type = RTS_TASK_CREATE_PART;
        rep.payload = rts_id;
        LOG("Task created with min budget. Res. id: %d\n", rts_id);
    }
    else 
    {
        rep.rep_type = RTS_TASK_CREATE_OK;
        rep.payload = rts_id;
        LOG("It is possible to guarantee these parameters. Res. id: %d\n", rts_id);
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
static struct rts_reply req_task_modify(struct rts_daemon* data, int cli_id) 
{
    int res;
    struct rts_reply rep;
    struct rts_request req;

    req = rts_carrier_get_req(&(data->chann), cli_id);
    
    LOG("Received RSV_MODIFY REQ for rsv: %d\n", req.payload.ids.rsvid);

    res = rts_scheduler_task_change(&(data->sched), &req.payload.param, req.payload.ids.rsvid);
    res = req.payload.ids.rsvid; // TO BE DELETED

    if(res == RTS_NO) 
    {
        rep.rep_type = RTS_TASK_MODIFY_ERR;
        LOG("It is NOT possible to guarantee these parameters!\n");
    }
    else if (res == RTS_PARTIAL)
    {
        rep.rep_type = RTS_TASK_MODIFY_PART;
        LOG("Reservation modified with min budget.\n");
    }
    else 
    {
        rep.rep_type = RTS_TASK_MODIFY_OK;
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
static struct rts_reply req_task_attach(struct rts_daemon* data, int cli_id) 
{
    struct rts_reply rep;
    struct rts_request req;

    req = rts_carrier_get_req(&(data->chann), cli_id);
    
    LOG("Received RSV_ATTACH REQ for res: %d. PID: %d will be attached.\n", 
            req.payload.ids.rsvid, req.payload.ids.pid);
    
    if(rts_scheduler_task_attach(&(data->sched), req.payload.ids.rsvid, req.payload.ids.pid) < 0)
        rep.rep_type = RTS_TASK_ATTACH_ERR;
    else
        rep.rep_type = RTS_TASK_ATTACH_OK;
    
    return rep;
}


/**
 * @internal
 * 
 * Client wants to detach the flow of execution from the reservation
 * 
 * @endinternal
 */
static struct rts_reply req_task_detach(struct rts_daemon* data, int cli_id) 
{
    struct rts_reply rep;
    struct rts_request req;

    req = rts_carrier_get_req(&(data->chann), cli_id);
    
    LOG("Received RSV_DETACH REQ for res: %d. The thread will be detached\n", 
        req.payload.ids.rsvid);
    
    if(rts_scheduler_task_detach(&(data->sched), req.payload.ids.rsvid) < 0)
        rep.rep_type = RTS_TASK_DETACH_ERR;
    else
        rep.rep_type = RTS_TASK_DETACH_OK;
    
    return rep;   
}

/**
 * @internal
 * 
 * Client wants to destroy given reservation
 * 
 * @endinternal
 */
static struct rts_reply req_task_destroy(struct rts_daemon* data, int cli_id) 
{
    struct rts_reply rep;
    struct rts_request req;

    req = rts_carrier_get_req(&(data->chann), cli_id);
    
    LOG("Received RSV_DESTROY REQ for res: %d. The thread will be detached\n", 
        req.payload.ids.rsvid);
    
    if(rts_scheduler_task_destroy(&(data->sched), req.payload.ids.rsvid) < 0)
        rep.rep_type = RTS_TASK_DESTROY_ERR;
    else
        rep.rep_type = RTS_TASK_DESTROY_OK;
    
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
int rts_daemon_check_for_fail(struct rts_daemon* data, int cli_id) 
{
    enum CLIENT_STATE st;
    pid_t pid;
    
    st = rts_carrier_get_state(&(data->chann), cli_id);
    
    if(st != ERROR && st != DISCONNECTED)
        return 0;
    
    INFO("Client %d disconnected. Its reservation will be destroyed.\n", cli_id);
    
    rts_carrier_set_state(&(data->chann), cli_id, EMPTY);
    pid = rts_carrier_get_pid(&(data->chann), cli_id);
    
    rts_scheduler_delete(&(data->sched), pid);
    
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
int rts_daemon_check_for_update(struct rts_daemon* data, int cli_id) 
{
    int is_updated;
    enum CLIENT_STATE st;
    
    is_updated = rts_carrier_is_updated(&(data->chann), cli_id);
    st = rts_carrier_get_state(&(data->chann), cli_id);
    
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
int rts_daemon_process_req(struct rts_daemon* data, int cli_id) 
{
    struct rts_reply rep;
    struct rts_request req;

    req = rts_carrier_get_req(&(data->chann), cli_id);
    
    switch(req.req_type) 
    {
        case RTS_CONNECTION:
            rep = req_connection(data, cli_id);
            break;
        case RTS_TASK_CREATE:
            rep = req_task_create(data, cli_id);
            break;
        case RTS_TASK_MODIFY:
            rep = req_task_modify(data, cli_id);
            break;
        case RTS_TASK_ATTACH:
            rep = req_task_attach(data, cli_id);
            break;
        case RTS_TASK_DETACH:
            rep = req_task_detach(data, cli_id);
            break;
        case RTS_TASK_DESTROY:
            rep = req_task_destroy(data, cli_id);
            break;
        default:
            rep.rep_type = RTS_REQUEST_ERR;
    }
    
    return rts_carrier_send(&(data->chann), &rep, cli_id);
}

/**
 * @internal
 * 
 * After checking connection status, if client has sent something
 * pass to process request
 * 
 * @endinternal
 */
void rts_daemon_handle_req(struct rts_daemon* data, int cli_id) 
{
    int sent;
        
    if(rts_daemon_check_for_fail(data, cli_id))
        return;
    
    if(!rts_daemon_check_for_update(data, cli_id))
        return;
    
    sent = rts_daemon_process_req(data, cli_id);
    rts_carrier_req_clear(&(data->chann), cli_id);

    if(sent <= 0)
        rts_carrier_set_state(&(data->chann), cli_id, ERROR);    
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
int rts_daemon_init(struct rts_daemon* data) 
{    
    if(rts_carrier_init(&(data->chann)) < 0)
        return -1;

    rts_taskset_init(&(data->tasks));
    
    if (rts_scheduler_init(&(data->sched), &(data->tasks)) < 0)
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
void rts_daemon_loop(struct rts_daemon* data) 
{    
    while(1) 
    {
        rts_carrier_update(&(data->chann));
        
        for(int i = 0; i <= rts_carrier_get_conn(&(data->chann)); i++)
            rts_daemon_handle_req(data, i);
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
void rts_daemon_dump(struct rts_daemon* data) 
{
    LOG("###### SYS INFO ######\n");
    rts_scheduler_dump(&(data->sched));
    LOG("######################\n");
    LOG("###### CONNECTION INFO ######\n");
    rts_carrier_dump(&(data->chann));
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
void rts_daemon_destroy(struct rts_daemon* data) 
{
    struct rts_task* t;
        
    while(1) 
    {
        t = rts_taskset_remove_top(&(data->tasks));
        
        if(t == NULL)
            break;
        
        rts_task_destroy(t);
    }
    
    rts_scheduler_destroy(&(data->sched));
}
