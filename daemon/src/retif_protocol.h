/**
 * @file retif_protocol.h
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
 * - retif_CONNECTION
 * - retif_RSV_CREATE
 * - retif_RSV_ATTACH
 * - retif_RSV_DETACH
 * - retif_RSV_QUERY
 * - retif_RSV_DESTROY
 * - retif_DECONNECTION
 *
 * #############################################################################
 * # Replies
 * #############################################################################
 *
 * - retif_REQUEST_ERR
 * - retif_CONNECTION_OK
 * - retif_CONNECTION_ERR
 * - retif_RSV_CREATE_OK
 * - retif_RSV_CREATE_MIN
 * - retif_RSV_CREATE_ERR
 * - retif_RSV_ATTACH_OK
 * - retif_RSV_ATTACH_ERR
 * - retif_RSV_DETACH_OK
 * - retif_RSV_DETACH_ERR
 * - retif_RSV_QUERY_OK
 * - retif_RSV_QUERY_ERR
 * - retif_RSV_DESTROY_OK
 * - retif_RSV_DESTROY_ERR
 * - retif_DECONNECTION_OK
 * - retif_DECONNECTION_ERR
 *
 * #############################################################################
 * # Behavior & details
 * #############################################################################
 *
 * ## retif_CONNECTION
 *
 * DESC:
 *  Client is requesting to connect to daemon services.
 * PARAM:
 *  Client process id
 * REPLIES:
 *  retif_CONNECTION_OK: Daemon accepts the connection.
 *  retif_CONNECTION_ERR: Error during connection.
 * PAYLOAD:
 *  Reply type
 *
 * ## retif_CAP_QUERY
 *
 * DESC:
 *  Client is asking info about current system budget
 * PARAM:
 *  SCHED_CLASSES: retif_BUDGET or retif_REMAINING_BUDGET
 * REPLIES:
 *  retif_CAP_QUERY_OK: System total rt utilization percentage
 *  retif_CAP_QUERY_OK: System current rt utilization percentage
 *  retif_CAP_QUERY_ERR: Wrong param type
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
 *  retif_params: budget, period, wcet, priority ..
 *  Client process id
 * REPLIES:
 *  retif_RSV_CREATE_ERR: Impossible to guarantee the request
 *  retif_RSV_CREATE_OK: Reservation created
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
 *  retif_RSV_ATTACH_ERR: Unable to attach this flow of execution
 *  retif_RSV_ATTACH_OK: Flow of execution attached
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
 *  retif_RSV_DETACH_ERR: Reservation not found
 *  retif_RSV_DETACH_OK: Detached with success
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
 *  retif_RSV_DESTROY_ERR: Reservation not found
 *  retif_RSV_DESTROY_OK: Destroyed with success
 * PAYLOAD:
 *  Reply type
 *
 *
 *
 */
