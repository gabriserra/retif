/**
 * @file usocket.h
 * @author Gabriele Serra
 * @date 13 Oct 2018
 * @brief Contains the interface of a unix socket IPC channel
 *
 * This module realizes a client/server IPC channel built on unix-domain sockets
 */

#ifndef USOCKET_H
#define USOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

// ---------------------------------------------
// DATA STRUCTURES
// ---------------------------------------------

#define TCP SOCK_STREAM
#define UDP SOCK_DGRAM

#define SELECT_MAX_WAIT 50
#define BACKLOG_MAX 128
#define SET_MAX_SIZE FD_SETSIZE

/**
 * @brief Contains descriptors needed to implement a client server communication
 * 
 * The structure usocket contains a unix domain socket descriptor,
 * the filepath string to which the socket is binded and a file-descriptor
 * sets that contains all sockets used to communicate with clients. The
 * structure can be used both by clients and servers. Client will use only the
 * single socket descriptor while server will use also the file descriptor set
 */
struct usocket {
    int socket;         /** Unix-domain socket descriptor */
    char* filepath;     /** Filepath string to which unix-socket is binded */
    int conn_set_max;   /** Current maximum number of descriptor */
    fd_set conn_set;    /** Set of descriptors to listen */
};

// ---------------------------------------------
// COMMON FOR CLIENT / SERVER
// ---------------------------------------------

/**
 * @brief Create an unix domain socket
 * 
 * Creates a server unix-domain-socket and initializes the communication
 * data structure with an empty fd-set and the given socket descriptor. 
 * Returns -1 if socket creation fails, 0 otherwise.
 * 
 * @param us pointer to usocket struct that will be initialized with sock info
 * @param socktype type of socket (available both TCP and UDP)
 * @return -1 in case of error, 0 if the operation was successful
 */
int usocket_init(struct usocket* us, int socktype);

// ---------------------------------------------
// SPECIFIC FOR CLIENTS
// ---------------------------------------------

int usocket_connect(struct usocket* us, char* filepath);

int usocket_recv(struct usocket* us, void* elem, size_t size);

int usocket_send(struct usocket* us, void* elem, size_t size);

// ---------------------------------------------
// SPECIFIC FOR SERVERS
// ---------------------------------------------

/**
 * @brief Binds the main unix socket to a file path
 * 
 * Binds the main unix socket to a file path, ensuring to free-up previously
 * reused filename. Returns 0 in case of success, -1 otherwise
 * 
 * @param us pointer to usocket struct that contains socket to bind
 * @param filepath path of filesystem to which bind the unix socket
 */
int usocket_bind(struct usocket* us, char* filepath);

/**
 * @brief Marks main server socket as awaiting for new connections
 * 
 * Marks server socket as awaiting for connections on binded filename. The 
 * maximum number of requests supported by the server is set to 128 by default 
 * but can be set with @p max_req arg. Returns 0 in case of success or -1 
 * otherwise
 * 
 * @param us pointer to usocket structure that contains the main socket
 * @param max_req maximum number of contemporary incoming new connection reqs
 * @return returns 0 in case of success or -1 otherwise
 */
int usocket_listen(struct usocket* us, int max_req);

/**
 * @brief Wait for incoming new connection and accept it
 * 
 * Put server waiting for incoming connection and, if will arrive, accepts a 
 * client request of connection and returns a new socket that can be used to 
 * communicate with the client. Return a descriptor in case of success or -1
 * otherwise
 * 
 * @param us pointer to usocket structure that contains the main socket
 * @return a new descriptor in case of success, -1 in case of errors
 */
int usocket_accept(struct usocket* us);

/**
 * @brief Recvs data from a connected client
 * 
 * Receives @p size data from one of the fd-set contained in @p us and copies it in 
 * @p elem buffer. Returns -1 in case of errors, 0 if client has closed the 
 * connection, a positive value that indicates the number of bytes received in 
 * case of success
 * 
 * @param us pointer to usocket structure that contains the descriptors set
 * @param elem pointer to elem to which copy data received
 * @param size number of bytes to receive
 * @param i the number of descriptor to receive data
 */
int usocket_recvfrom(struct usocket* us, void* elem, size_t size, int i);

/**
 * @brief Sends data to a connected client
 * 
 * Sends @p size data to one of the fd-set contained in @p us copying from @p elem
 * buffer. Returns -1 in case of errors, 0 if client has closed the connection,
 * a positive value that indicates the number of bytes sent in case of 
 * success
 * 
 * @param us pointer to usocket structure that contains the descriptors set
 * @param elem pointer to elem from which copy data to send off
 * @param size number of bytes to send
 * @param i the number of descriptor to send data
 */
int usocket_sendto(struct usocket* us, void* elem, size_t size, int i);

/**
 * @brief Sets the main socket as non-blocking
 * 
 * Sets the main socket of @p us as a non-blocking socket.
 * 
 * @param us pointer to usocket structure that contains the main socket
 * @return 0 in case of success, -1 otherwise
 */
int usocket_nonblock(struct usocket* us);

/**
 * @brief Sets the main socket as blocking
 * 
 * Sets the main socket of @p us as a blocking socket.
 * 
 * @param us pointer to usocket structure that contains the main socket
 * @return 0 in case of success, -1 otherwise
 */
int usocket_block(struct usocket* us);

/**
 * @brief Sets a timeout for operations on main socket
 * 
 * Sets a timeout specified in @p ms both for send and receive operations
 * 
 * @param us pointer to usocket structure that contains the main socket
 * @param ms number of milliseconds of timeout 
 * @return 0 if timeouts are set with success, -1 otherwise
 */
int usocket_timeout(struct usocket* us, int ms);

/**
 * @brief Get the current max number of descriptor in set
 * 
 * Gets the current max number of descriptor in fd-set. In case fd-set is
 * empty or someway the greater descriptor is lower than the main socket,
 * the main socket descriptor number is returned.
 * 
 * @param us pointer to usocket structure that contains the set
 * @return an integer number representing the maximum descriptor number
 */
int usocket_get_maxfd(struct usocket* us);

/**
 * @brief Initializes the descriptor set
 * 
 * Initialize the fd-set in @p us and place the main socket into the set.
 * 
 * @param us pointer to usocket structure that contains the empty set
 */
void usocket_prepare_recv(struct usocket* us);

/**
 * @brief Put server waiting for communication and return received data
 * 
 * Put the server waiting for data on fd-set and wakes up the server if one
 * or more client has been communicating with the server. For each client that
 * has been sent data, place @p size data (if available) in the the buffer @p data.
 * Also returns the number of byte received for all client in @p nrecv.
 * Returns 0 in case of success, -1 if errors occurred
 * 
 * @param us pointer to structure that contains descriptors set
 * @param data pointer to array of generic type used to store received data
 * @param nrecv array of integers that will contain the number of bytes received
 * @size maximum number of byte receiveable from each descriptor
 * @return -1 in case of errors, 0 in case of success
 */
int usocket_recvall(struct usocket* us, void* data, int nrecv[SET_MAX_SIZE], size_t size);

/**
 * @brief Accepts a new connection and updates fd set
 * 
 * Accept a new connection and add the communication description to the
 * fd set of @p us. Return the socket descriptor number in case of success or
 * -1 in case of error.
 * 
 * @param us pointer to usocket structure
 * @return -1 in case of errors, the new socket descriptor otherwise
 */
int usocket_add_connections(struct usocket* us);

/**
 * @brief Removes given descriptor
 *
 * Removes given @p fd from the fd-set of @p us 
 * 
 * @param us pointer to usocket structure that contains fd-set to modify
 * @param fd descriptor number that will be removed
 */
void usocket_remove_connection(struct usocket* us, int fd);

#endif
