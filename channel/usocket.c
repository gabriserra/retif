/**
 * @file usocket.c
 * @author Gabriele Serra
 * @date 10 Nov 2018
 * @brief Contains the implementation of a client/server unix socket IPC channel
 *
 * This module realizes a client/server IPC channel built on unix-domain sockets
 */

#include "usocket.h"
#include "logger.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

// ---------------------------------------------
// COMMON FOR CLIENT / SERVER
// ---------------------------------------------

/**
 * @internal
 *
 * Creates a server unix-domain-socket and initializes the communication
 * data structure with an empty fd-set and the given socket descriptor.
 * Returns -1 if socket creation fails, 0 otherwise.
 *
 * @endinternal
 */
int usocket_init(struct usocket *us, int socktype)
{
    int sock;

    memset(us, 0, sizeof(struct usocket));
    sock = socket(AF_UNIX, socktype, 0);

    if (sock < 0)
        return -1;

    us->socket = sock;
    us->conn_set_max = 0;
    us->filepath = NULL;
    FD_ZERO(&(us->conn_set));
    return 0;
}

// ---------------------------------------------
// SPECIFIC FOR CLIENTS
// ---------------------------------------------

/**
 * @internal
 *
 * Connects the socket contained in @p us to the given filepath. Returns 0 in
 * of success, -1 in case of errors.
 *
 * @endinternal
 */
int usocket_connect(struct usocket *us, char *filepath)
{
    struct sockaddr_un usock_sockaddr;

    memset(&usock_sockaddr, 0, sizeof(struct sockaddr_un));
    usock_sockaddr.sun_family = AF_UNIX;
    strcpy(usock_sockaddr.sun_path, filepath);
    return connect(us->socket, (struct sockaddr *) &usock_sockaddr,
        sizeof(struct sockaddr_un));
}

/**
 * @internal
 *
 * Receives @p size data from the socket contained in @p us and copies it in @p
 * elem buffer. Returns -1 in case of errors, 0 if server has closed the
 * connection, a positive value that indicates the number of bytes received in
 * case of success
 *
 * @endinternal
 */
int usocket_recv(struct usocket *us, void *elem, size_t size)
{
    return recv(us->socket, elem, size, 0);
}

/**
 * @internal
 *
 * Sends @p size data to the socket contained in @p us copying from @p elem
 * buffer. Returns -1 in case of errors, 0 if server has closed the connection,
 * a positive value that indicates the number of bytes sent in case of
 * success
 *
 * @endinternal
 */
int usocket_send(struct usocket *us, void *elem, size_t size)
{
    return send(us->socket, elem, size, 0);
}

// ---------------------------------------------
// SPECIFIC FOR SERVERS
// ---------------------------------------------

/**
 * @internal
 *
 * Binds the main unix socket to a file path, ensuring to free-up previously
 * reused filename. Returns 0 in case of success, -1 otherwise
 *
 * @endinternal
 */
int usocket_bind(struct usocket *us, char *filepath)
{
    struct sockaddr_un usock_sockaddr;
    int ret;

    memset(&usock_sockaddr, 0, sizeof(struct sockaddr_un));
    usock_sockaddr.sun_family = AF_UNIX;
    strcpy(usock_sockaddr.sun_path, filepath);
    unlink(filepath);

    ret = bind(us->socket, (struct sockaddr *) &usock_sockaddr,
        sizeof(struct sockaddr_un));

    if (ret < 0)
        return -1;

    return chmod(filepath, S_IRWXU | S_IRWXG | S_IRWXO);
}

/**
 * @internal
 *
 * Marks server socket as awaiting for connections on binded filename. The
 * maximum number of requests supported by the server is set to 128 by default
 * but can be set with \max_req arg. Returns 0 in case of success or -1
 * otherwise
 *
 * @endinternal
 */
int usocket_listen(struct usocket *us, int max_req)
{
    if (!max_req)
        max_req = BACKLOG_MAX;

    return listen(us->socket, max_req);
}

/**
 * @internal
 *
 * Put server waiting for incoming connection and, if will arrive, accepts a
 * client request of connection and returns a new socket that can be used to
 * communicate with the client. Return a descriptor in case of success or -1
 * otherwise
 *
 * @endinternal
 */
int usocket_accept(struct usocket *us)
{
    struct sockaddr_un client;
    socklen_t struct_len;

    struct_len = sizeof(struct sockaddr_un);
    return accept(us->socket, (struct sockaddr *) &client, &struct_len);
}

/**
 * @internal
 *
 * Receives @p size data from one of the fd-set contained in @p us and copies it
 * in
 * @p elem buffer. Returns -1 in case of errors, 0 if client has closed the
 * connection, a positive value that indicates the number of bytes received in
 * case of success
 *
 * @endinternal
 */
int usocket_recvfrom(struct usocket *us, void *elem, size_t size, int i)
{
    return recv(i, elem, size, 0);
}

/**
 * @internal
 *
 * Sends @p size data from one of the fd-set contained in @p us copying from @p
 * elem buffer. Returns -1 in case of errors, 0 if client has closed the
 * connection, a positive value that indicates the number of bytes sent in case
 * of success
 *
 * @endinternal
 */
int usocket_sendto(struct usocket *us, void *elem, size_t size, int i)
{
    return send(i, elem, size, 0);
}

/**
 * @internal
 *
 * Sets the main socket of @p us as a non-blocking socket.
 *
 * @endinternal
 */
int usocket_nonblock(struct usocket *us)
{
    return fcntl(us->socket, F_SETFL, O_NONBLOCK);
}

/**
 * @internal
 *
 * Sets the main socket of @p us as a blocking socket.
 *
 * @endinternal
 */
int usocket_block(struct usocket *us)
{
    return fcntl(us->socket, F_SETFL, ~O_NONBLOCK);
}

/**
 * @internal
 *
 * Sets a timeout specified in @p ms both for send and receive operation
 *
 * @endinternal
 */
int usocket_timeout(struct usocket *us, int ms)
{
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;

    if (setsockopt(us->socket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &tv,
            sizeof tv) < 0)
        return -1;
    if (setsockopt(us->socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv,
            sizeof tv) < 0)
        return -1;

    return 0;
}

/**
 * @internal
 *
 * Gets the current max number of descriptor in fd-set. In case fd-set is
 * empty or someway the greater descriptor is lower than the main socket,
 * the main socket descriptor number is returned.
 *
 * @endinternal
 */
int usocket_get_maxfd(struct usocket *us)
{
    return us->conn_set_max > us->socket ? us->conn_set_max : us->socket;
}

/**
 * @internal
 *
 * Initialize the fd-set in @p us and place the main socket into the set.
 *
 * @endinternal
 */
void usocket_prepare_recv(struct usocket *us)
{
    FD_ZERO(&(us->conn_set));
    FD_SET(us->socket, &(us->conn_set));
    us->conn_set_max = us->socket;
}

/**
 * @internal
 *
 * Put the server waiting for data on fd-set and wakes up the server if one
 * or more client has been communicating with the server. For each client that
 * has been sent data, place @p size data (if available) in the the buffer @p
 * data. Also returns the number of byte received for all client in @p nrecv.
 * Returns 0 in case of success, -1 if errors occurred
 *
 * @endinternal
 */
int usocket_recvall(struct usocket *us, void *data, int nrecv[SET_MAX_SIZE],
    size_t size)
{
    int i;
    fd_set temp_conn_set;

    FD_ZERO(&temp_conn_set);
    temp_conn_set = us->conn_set;

    if (select(us->conn_set_max + 1, &temp_conn_set, 0, 0, 0) < 0)
        return -1;

    for (i = 0; i <= us->conn_set_max; i++)
    {
        if (!FD_ISSET(i, &temp_conn_set))
            continue;

        if (i == us->socket)
        {
            nrecv[i] = usocket_add_connections(us);
            continue;
        }

        nrecv[i] = recv(i, data + (i * size), size, 0);

        if (!nrecv[i])
        {
            nrecv[i] = -1;
            FD_CLR(i, &(us->conn_set));
        }
    }

    return 0;
}

/**
 * @internal
 *
 * Accept a new connection and add the communication description to the
 * fd set of @p us. Return the socket descriptor number in case of success or
 * -1 in case of error.
 *
 * @endinternal
 */
int usocket_add_connections(struct usocket *us)
{
    int newfd = usocket_accept(us);

    if (newfd < 0)
        return -1;

    if (usocket_get_credentials(us, newfd) < 0)
        return -1;

    FD_SET(newfd, &(us->conn_set));
    us->conn_set_max = us->conn_set_max > newfd ? us->conn_set_max : newfd;

    return newfd;
}

/**
 * @internal
 *
 * Removes given @p fd from the fd-set of @p us
 *
 * @endinternal
 */
void usocket_remove_connection(struct usocket *us, int fd)
{
    FD_CLR(fd, &(us->conn_set));
    close(fd);
}

/**
 * @internal
 *
 * Get connected client credentials and stores it
 *
 * @endinternal
 */
int usocket_get_credentials(struct usocket *us, int fd)
{
    struct ucred *ucredp;
    socklen_t len = sizeof(struct ucred);

    ucredp = calloc(1, len);

    if (ucredp == NULL)
    {
        LOG(ERR, "Unable to allocate ucred struct. %s. \n", strerror(errno));
        return -1;
    }

    // get client credentials
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, ucredp, &len) == -1)
    {
        LOG(ERR, "Unable to get credentials: %s. \n", strerror(errno));
        return -1;
    }

    LOG(INFO, "Credentials from %d descriptor: (%d, %d, %d)\n", fd, ucredp->pid,
        ucredp->uid, ucredp->gid);
    us->ucredp[fd] = ucredp;

    return 0;
}
