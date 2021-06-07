#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "logger.h"
#include "retif_channel.h"


// -----------------------------------------------------------------------------
// CHANNEL ACCESS METHODS
// -----------------------------------------------------------------------------

int retif_access_init(struct retif_access* c)
{
    if(usocket_init(&(c->sock), TCP) < 0)
        return -1;

    return 0;
}

int retif_access_connect(struct retif_access* c)
{
    return usocket_connect(&(c->sock), CHANNEL_PATH_ACCESS);
}

int retif_access_recv(struct retif_access* c)
{
    return usocket_recv(&(c->sock), (void *)&(c->rep), sizeof(struct retif_reply));
}

int retif_access_send(struct retif_access* c)
{
    return usocket_send(&(c->sock), (void *)&(c->req), sizeof(struct retif_request));
}

// -----------------------------------------------------------------------------
// CHANNEL CARRIER METHODS
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * Initializes the server side of channel binding the socket on
 * defined PATH.
 *
 * @endinternal
 */
int retif_carrier_init(struct retif_carrier* c)
{
    memset(c, 0, sizeof(struct retif_access));

    if(usocket_init(&(c->sock), TCP) < 0)
        return -1;

    if(usocket_bind(&(c->sock), CHANNEL_PATH_CARRIER) < 0)
    {
        ERR("Unable to bind communication socket: %s\n", strerror(errno));
        return -1;
    }

    if(usocket_listen(&(c->sock), BACKLOG_MAX) < 0)
        return -1;

    usocket_prepare_recv(&(c->sock));

    return 0;
}

/**
 * @internal
 *
 * Receives new data from clients and updates their current states following
 * this FSM schema:
 *  main socket --> do nothing, incoming connection are handled internally
 *  empty descriptor and no data received --> nothing to do
 *  empty descriptor but some data received --> a new client is CONNECTED
 *  connected client and data received --> do nothing, daemon will react
 *  receive error --> set as ERROR
 *  receive disconnection --> set as DISCONNECTED
 *
 * @endinternal
 */
void retif_carrier_update(struct retif_carrier* c)
{
    int i, n;

    n = usocket_get_maxfd(&(c->sock));
    memset(&(c->last_n), 0, n+1);

    if (usocket_recvall(&(c->sock), (void*)&(c->last_req), (int*)&(c->last_n), sizeof(struct retif_request)) < 0)
        return;

    for(i = 0; i <= n; i++)
    {
        if(i == c->sock.socket)
            continue;
        else if(c->client[i].state == EMPTY && c->last_n[i] == 0)
            continue;
        else if(c->client[i].state == EMPTY && c->last_n[i] != 0)
            c->client[i].state = CONNECTED;
        else if(c->client[i].state == CONNECTED && c->last_n[i] > 0)
            continue;
        else if(c->last_n[i] < 0)
            c->client[i].state = ERROR;
        else
            c->client[i].state = DISCONNECTED;
    }
}

/**
 * @internal
 *
 * Sends a reply packet to a client associated with descriptor @p cli_id
 *
 * @endinternal
 */
int retif_carrier_send(struct retif_carrier* c, struct retif_reply* r, int cli_id)
{
    return usocket_sendto(&(c->sock), (void*)r, sizeof(struct retif_reply), cli_id);
}

/**
 * @internal
 *
 * Returns the highest number of valid client connection among all
 * the possible set of descriptor
 *
 * @endinternal
 */
int retif_carrier_get_conn(struct retif_carrier* c)
{
    return usocket_get_maxfd(&(c->sock));
}

/**
 * @internal
 *
 * Returns the last request made by the client associated with descriptor @p cli_id
 *
 * @endinternal
 */
struct retif_request retif_carrier_get_req(struct retif_carrier* c, int cli_id)
{
    return c->last_req[cli_id];
}

/**
 * @internal
 *
 * Checks if the client associated with descriptor @p cli_id has been sent
 * a request during the last daemon loop
 *
 * @endinternal
 */
int retif_carrier_is_updated(struct retif_carrier* c, int cli_id)
{
    if(c->last_n[cli_id] > 0)
        return 1;

    return 0;
}

/**
 * @internal
 *
 * Clears the request of @p cli_id setting last num of bytes received to 0
 *
 * @endinternal
 */
void retif_carrier_req_clear(struct retif_carrier* c, int cli_id)
{
    c->last_n[cli_id] = 0;
}

/**
 * @internal
 *
 * Returns the current state of the client associated with descriptor @p cli_id
 *
 * @endinternal
 */
enum CLIENT_STATE retif_carrier_get_state(struct retif_carrier* c, int cli_id)
{
    return c->client[cli_id].state;
}

/**
 * @internal
 *
 * Sets the current state of the client associated with descriptor @p cli_id
 *
 * @endinternal
 */
void retif_carrier_set_state(struct retif_carrier* c, int cli_id, enum CLIENT_STATE s)
{
    c->client[cli_id].state = s;
}

/**
 * @internal
 *
 * Returns the pid of the client associated with descriptor @p cli_id
 *
 * @endinternal
 */
pid_t retif_carrier_get_pid(struct retif_carrier* c, int cli_id)
{
    return c->client[cli_id].pid;
}

/**
 * @internal
 *
 * Sets the pid of the client associated with descriptor @p cli_id
 *
 * @endinternal
 */
void retif_carrier_set_pid(struct retif_carrier* c, int cli_id, pid_t pid)
{
    c->client[cli_id].pid = pid;
}

/**
 * @internal
 *
 * Dump the connection state and PID of each client connected
 *
 * @endinternal
 */
void retif_carrier_dump(struct retif_carrier* c)
{
    struct retif_client* client;

    for (int i = 0; i < CHANNEL_MAX_SIZE; i++)
    {
        client = &(c->client[i]);

        if (client->pid == 0)
            continue;

        LOG("-> Client %d - PID: %d - STATE: %d\n", i, client->pid, client->state);
    }
}
