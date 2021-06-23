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

#ifndef RETIF_DAEMON_H
#define RETIF_DAEMON_H

#include "retif_channel.h"
#include "retif_config.h"
#include "retif_scheduler.h"
#include "retif_taskset.h"

/**
 * @brief Main daemon data structure
 *
 * Main daemon data structure, contains the 'carrier' server part of channel
 * access, the scheduler object (the 'logic' for scheduling) and the entire
 * taskset with currently served task
 */
struct rtf_daemon
{
    struct proc_backup proc_backup;
    configuration_t config;
    struct rtf_carrier chann;
    struct rtf_scheduler sched;
    struct rtf_taskset tasks;
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
int rtf_daemon_init(struct rtf_daemon *data);

/**
 * @brief Realizes daemon loop
 *
 * Realizes daemon loop, waiting for requests and handling it
 *
 * @param data main daemon data structure
 */
void rtf_daemon_loop(struct rtf_daemon *data);

/**
 * @brief Dumps all daemon info
 *
 * Dumps all daemon data info such as connected proccesses,
 * tasks, scheduling plugins and so on.
 *
 * @param data main daemon data structure
 */
void rtf_daemon_dump(struct rtf_daemon *data);

/**
 * @brief Tear down daemon safely
 *
 * Stops all services of daemon safely, freeing memory,
 * restoring kernel params and stopping loop.
 *
 * @param data main daemon data structure
 */
void rtf_daemon_destroy(struct rtf_daemon *data);

#endif /* RETIF_DAEMON_H */
