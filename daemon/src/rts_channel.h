/**
 * @file rts_config.h
 * @author Gabriele Serra
 * @date 06 Jan 2020
 * @brief IPC abstraction level for real-time-support daemon
 * 
 * The module is composed of 2 parts, the channel endpoint for clients
 * (processes that require the service) and the channel endpoint for the
 * server (namely the daemon). The client channel service is called 'access'
 * while the server channel service is called 'carrier' because it realizes
 * the actual service support. Due to this choice, functions for clients are
 * prefixed with 'access' and viceversa functions for server are prefixed with
 * 'carrier'. 
 */

#ifndef RTS_CHANNEL_H
#define RTS_CHANNEL_H

#include "usocket.h"
#include "rts_types.h"

#define CHANNEL_PATH_CARRIER    "/tmp/rtschan"
#define CHANNEL_PATH_ACCESS     "/tmp/rtschan"
#define CHANNEL_MAX_SIZE        SET_MAX_SIZE

/**
 * @brief Data-structure that each client must use to communicate with daemon
 */
struct rts_access {
    struct usocket sock;
    struct rts_request req;
    struct rts_reply rep;
};

/**
 * @brief Data-structure that the server use to keep info about clients
 */
struct rts_carrier {
    struct usocket sock;
    int last_n[CHANNEL_MAX_SIZE];
    struct rts_request last_req[CHANNEL_MAX_SIZE];
    struct rts_client client[CHANNEL_MAX_SIZE];
};

// -----------------------------------------------------------------------------
// CHANNEL ACCESS METHODS
// -----------------------------------------------------------------------------

int rts_access_init(struct rts_access* c);

int rts_access_connect(struct rts_access* c);

int rts_access_recv(struct rts_access* c);

int rts_access_send(struct rts_access* c);

// -----------------------------------------------------------------------------
// CHANNEL CARRIER METHODS
// -----------------------------------------------------------------------------

/**
 * @brief Initializes the server side of the channels
 * 
 * Initializes the server side of channel binding the socket on
 * defined PATH.
 * 
 * @param c pointer to channel data structure of the daemon
 * @return -1 in case of errors or 0 otherwise
 */
int rts_carrier_init(struct rts_carrier* c);

/**
 * @brief Receives new data from clients
 * 
 * Receives new data from clients and updates their current states
 * 
 * @param c pointer to channel data structure of the daemon
 */
void rts_carrier_update(struct rts_carrier* c);

/**
 * @brief Sends a reply to a client
 * 
 * Sends a reply packet to a client associated with descriptor @p cli_id and
 * returns -1 in case of errors, 0 if client has closed the connection,
 * a positive value that indicates the number of bytes sent in case of 
 * success
 * 
 * @param c pointer to channel data structure of the daemon
 * @param rep pointer to a reply packet that will be sent to the client
 * @param cli_id id descriptor of the client
 * @return the number of byte sent in case of success
 */
int rts_carrier_send(struct rts_carrier* c, struct rts_reply* rep, int cli_id);

/**
 * @brief Returns the highest number of client connection descriptor
 * 
 * Return the highest number of valid client connection among all
 * the possible set of descriptor
 * 
 * @param c pointer to channel data structure of the daemon
 * @return the highest number of descriptor [0-SET_MAX_SIZE] 
 */
int rts_carrier_get_conn(struct rts_carrier* c);

/**
 * @brief Returns the last request made by the client
 * 
 * Returns the last request made by the client associated with descriptor 
 * @p cli_id, as a structure of type rts_request.
 * 
 * @param c pointer to channel data structure of the daemon
 * @param cli_id id descriptor of the client
 * @return copy of request data structure
 */
struct rts_request rts_carrier_get_req(struct rts_carrier* c, int cli_id);

/**
 * @brief Checks if given client has sent something
 * 
 * Checks if the client associated with descriptor @p cli_id has been sent
 * a request during the last daemon loop. Returns 1 if something was sent
 * or 0 otherwise
 * 
 * @param c pointer to channel data structure of the daemon
 * @param cli_id id descriptor of the client
 * @return 1 if something was sent, 0 otherwise 
 */
int rts_carrier_is_updated(struct rts_carrier* c, int cli_id);

/**
 * @brief Returns the current state of the client 
 * 
 * Returns the current state of the client associated with descriptor @p cli_id
 * regarding its connection with the server
 * 
 * @param c pointer to channel data structure of the daemon
 * @param cli_id id descriptor of the client
 * @return enum that describe client state (EMPTY, CONNECTED, DISCONNECTED, ERR)
 */
enum CLIENT_STATE rts_carrier_get_state(struct rts_carrier* c, int cli_id);

/**
 * @brief Sets the current state of the client
 * 
 * Sets the current state of the client associated with descriptor @p cli_id
 * regarding its connection with the server
 * 
 * @param c pointer to channel data structure of the daemon
 * @param cli_id id descriptor of the client
 * @param s describe client state (EMPTY, CONNECTED, DISCONNECTED, ERR)
 */
void rts_carrier_set_state(struct rts_carrier* c, int cli_id, enum CLIENT_STATE s);

/**
 * @brief Returns the pid of the client
 * 
 * Returns the pid of the client associated with descriptor @p cli_id
 * 
 * @param c pointer to channel data structure of the daemon
 * @param cli_id id descriptor of the client
 * @return pid of the client
 */
pid_t rts_carrier_get_pid(struct rts_carrier* c, int cli_id);

/**
 * @brief Sets the pid of the client
 * 
 * Sets the pid of the client associated with descriptor @p cli_id
 * 
 * @param c pointer to channel data structure of the daemon
 * @param cli_id id descriptor of the client
 * @param pid pid of the client that will be associated with the descriptor
 */
void rts_carrier_set_pid(struct rts_carrier* c, int cli_id, pid_t pid);

/**
 * @brief Dumps the connection info
 * 
 * Dump the connection state and PID of each client connected
 * 
 * @param c pointer to channel data structure of the daemon
 */
void rts_carrier_dump(struct rts_carrier* c);

#endif	// RTS_CHANNEL_H
