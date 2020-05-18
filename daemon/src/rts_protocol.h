/**
 * @file rts_protocol.h
 * @author Gabriele Serra
 * @date 07 Jan 2020
 * @brief Clients/Daemon protocol for RTS declarative support
 * 
 * This header file contains a pseudo-formal specification of the protocol
 * that lies under this framework. At first, there is a list of possible
 * requests that could be made by clients and a list of possible replies from
 * the daemon. Subsequently, for each type of requests, are listed all
 * characteristics such as the request param and explanation.
 */
/**
 * #############################################################################
 * # RTS - Protocol specification
 * #############################################################################
 * 
 * #############################################################################
 * # Requests
 * #############################################################################
 * 
 * - RTS_CONNECTION
 * - RTS_RSV_CREATE
 * - RTS_RSV_ATTACH
 * - RTS_RSV_DETACH
 * - RTS_RSV_QUERY
 * - RTS_RSV_DESTROY
 * - RTS_DECONNECTION
 *
 * #############################################################################
 * # Replies
 * #############################################################################
 * 
 * - RTS_REQUEST_ERR
 * - RTS_CONNECTION_OK
 * - RTS_CONNECTION_ERR
 * - RTS_RSV_CREATE_OK
 * - RTS_RSV_CREATE_MIN
 * - RTS_RSV_CREATE_ERR
 * - RTS_RSV_ATTACH_OK
 * - RTS_RSV_ATTACH_ERR
 * - RTS_RSV_DETACH_OK
 * - RTS_RSV_DETACH_ERR
 * - RTS_RSV_QUERY_OK
 * - RTS_RSV_QUERY_ERR
 * - RTS_RSV_DESTROY_OK
 * - RTS_RSV_DESTROY_ERR
 * - RTS_DECONNECTION_OK
 * - RTS_DECONNECTION_ERR
 * 
 * #############################################################################
 * # Behavior & details
 * #############################################################################
 * 
 * ## RTS_CONNECTION
 * 
 * DESC:
 *  Client is requesting to connect to daemon services.
 * PARAM:
 *  Client process id
 * REPLIES:
 *  RTS_CONNECTION_OK: Daemon accepts the connection.
 *  RTS_CONNECTION_ERR: Error during connection.
 * PAYLOAD:
 *  Reply type
 * 
 * ## RTS_CAP_QUERY
 * 
 * DESC:
 *  Client is asking info about current system budget
 * PARAM:
 *  SCHED_CLASSES: RTS_BUDGET or RTS_REMAINING_BUDGET
 * REPLIES:
 *  RTS_CAP_QUERY_OK: System total rt utilization percentage
 *  RTS_CAP_QUERY_OK: System current rt utilization percentage
 *  RTS_CAP_QUERY_ERR: Wrong param type
 * PAYLOAD:
 *  Reply type & Total free rt util
 *  Reply type & Remaining rt util
 *  Reply type & Null
 * 
 * ## REQ_RSV_CREATE
 * 
 * DESC:
 *  Client wants to create a reservation
 * PARAM:
 *  rts_params: budget, period, wcet, priority ..
 *  Client process id
 * REPLIES:
 *  RTS_RSV_CREATE_ERR: Impossible to guarantee the request
 *  RTS_RSV_CREATE_OK: Reservation created
 * PAYLOAD:
 *  Reply type & -1
 *  Reply tyoe & Reservation id
 * 
 * ## REQ_RSV_ATTACH
 * 
 * DESC:
 *  Client wants to attach a flow of execution to the reservation
 * PARAM:
 *  Reservation id
 * REPLIES:
 *  RTS_RSV_ATTACH_ERR: Unable to attach this flow of execution
 *  RTS_RSV_ATTACH_OK: Flow of execution attached
 * PAYLOAD:
 *  Reply type
 * 
 * ## REQ_RSV_DETACH
 *  
 * DESC:
 *  Client wants to detach the flow of execution from the reservation
 * PARAM:
 *  Reservation id
 * REPLIES:
 *  RTS_RSV_DETACH_ERR: Reservation not found
 *  RTS_RSV_DETACH_OK: Detached with success
 * PAYLOAD:
 *  Reply type 
 * 
 * ## REQ_RSV_DESTROY
 *  
 * DESC:
 *  Client wants to destroy given reservation
 * PARAM:
 *  Reservation id
 * REPLIES:
 *  RTS_RSV_DESTROY_ERR: Reservation not found
 *  RTS_RSV_DESTROY_OK: Destroyed with success
 * PAYLOAD:
 *  Reply type 
 * 
 *   
 * 
 */