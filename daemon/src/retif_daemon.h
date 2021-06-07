/**
 * @file retif_daemon.h
 * @author Gabriele Serra
 * @date 06 Jan 2020
 * @brief Exposes main daemon routine
 *
 * This library exposes main daemon routine: init, loop and destroy.
 * The aforementioned routines are used to start daemon services, process
 * and execute clients requests and stop the daemon lifecycle.
 */

#ifndef retif_DAEMON_H
#define retif_DAEMON_H

#include "retif_taskset.h"
#include "retif_channel.h"
#include "retif_scheduler.h"

/**
 * @brief Main daemon data structure
 *
 * Main daemon data structure, contains the 'carrier' server part of channel
 * access, the scheduler object (the 'logic' for scheduling) and the entire
 * taskset with currently served task
 */
struct retif_daemon
{
    struct retif_carrier chann;
    struct retif_scheduler sched;
    struct retif_taskset tasks;
};

/**
 * @brief Initializes daemon data structure
 *
 * Initializes daemon applying custom configuration and initializing carrier,
 * taskset & scheduler.
 *
 * @param data main daemon data structure
 * @return 0 in case of success, -1 otherwise
 */
int retif_daemon_init(struct retif_daemon* data);

/**
 * @brief Realizes daemon loop
 *
 * Realizes daemon loop, waiting for requests and handling it
 *
 * @param data main daemon data structure
 */
void retif_daemon_loop(struct retif_daemon* data);

/**
 * @brief Dumps all daemon info
 *
 * Dumps all daemon data info such as connected proccesses,
 * tasks, scheduling plugins and so on.
 *
 * @param data main daemon data structure
 */
void retif_daemon_dump(struct retif_daemon* data);

/**
 * @brief Tear down daemon safely
 *
 * Stops all services of daemon safely, freeing memory,
 * restoring kernel params and stopping loop.
 *
 * @param data main daemon data structure
 */
void retif_daemon_destroy(struct retif_daemon* data);

#endif /* retif_DAEMON_H */
