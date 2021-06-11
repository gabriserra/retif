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
 * - RTF_CONNECTION
 * - RTF_RSV_CREATE
 * - RTF_RSV_ATTACH
 * - RTF_RSV_DETACH
 * - RTF_RSV_QUERY
 * - RTF_RSV_DESTROY
 * - RTF_DECONNECTION
 *
 * #############################################################################
 * # Replies
 * #############################################################################
 *
 * - RTF_REQUEST_ERR
 * - RTF_CONNECTION_OK
 * - RTF_CONNECTION_ERR
 * - RTF_RSV_CREATE_OK
 * - RTF_RSV_CREATE_MIN
 * - RTF_RSV_CREATE_ERR
 * - RTF_RSV_ATTACH_OK
 * - RTF_RSV_ATTACH_ERR
 * - RTF_RSV_DETACH_OK
 * - RTF_RSV_DETACH_ERR
 * - RTF_RSV_QUERY_OK
 * - RTF_RSV_QUERY_ERR
 * - RTF_RSV_DESTROY_OK
 * - RTF_RSV_DESTROY_ERR
 * - RTF_DECONNECTION_OK
 * - RTF_DECONNECTION_ERR
 *
 * #############################################################################
 * # Behavior & details
 * #############################################################################
 *
 * ## RTF_CONNECTION
 *
 * DESC:
 *  Client is requesting to connect to daemon services.
 * PARAM:
 *  Client process id
 * REPLIES:
 *  RTF_CONNECTION_OK: Daemon accepts the connection.
 *  RTF_CONNECTION_ERR: Error during connection.
 * PAYLOAD:
 *  Reply type
 *
 * ## RTF_CAP_QUERY
 *
 * DESC:
 *  Client is asking info about current system budget
 * PARAM:
 *  SCHED_CLASSES: RTF_BUDGET or RTF_REMAINING_BUDGET
 * REPLIES:
 *  RTF_CAP_QUERY_OK: System total rt utilization percentage
 *  RTF_CAP_QUERY_OK: System current rt utilization percentage
 *  RTF_CAP_QUERY_ERR: Wrong param type
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
 *  rtf_params: budget, period, wcet, priority ..
 *  Client process id
 * REPLIES:
 *  RTF_RSV_CREATE_ERR: Impossible to guarantee the request
 *  RTF_RSV_CREATE_OK: Reservation created
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
 *  RTF_RSV_ATTACH_ERR: Unable to attach this flow of execution
 *  RTF_RSV_ATTACH_OK: Flow of execution attached
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
 *  RTF_RSV_DETACH_ERR: Reservation not found
 *  RTF_RSV_DETACH_OK: Detached with success
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
 *  RTF_RSV_DESTROY_ERR: Reservation not found
 *  RTF_RSV_DESTROY_OK: Destroyed with success
 * PAYLOAD:
 *  Reply type
 *
 *
 *
 */
