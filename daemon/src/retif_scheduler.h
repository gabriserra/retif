#ifndef RETIF_SCHEDULER_H
#define RETIF_SCHEDULER_H

#include <sys/types.h>
#include "retif_types.h"
#include "retif_plugin.h"

#define CLK             CLOCK_MONOTONIC

struct retif_taskset;
struct retif_task;

struct retif_scheduler
{
    int                     num_of_cpu;
    int                     num_of_plugins;
    long                    last_task_id;
    struct retif_taskset*     taskset;
    struct retif_plugin*      plugin;
};

/**
 * @brief Initializes all data needed for scheduler
 *
 * Initializes all data needed for scheduler (configuration & plugins)
 *
 * @param s pointer to scheduler data struct
 * @param ts pointer to taskset data struct
 */
int retif_scheduler_init(struct retif_scheduler* s, struct retif_taskset* ts);

/**
 * @brief Deallocate scheduler stuff
 *
 * Frees scheduler allocated stuff and restore params to default
 *
 * @param s pointer to scheduler data struct
 */
void retif_scheduler_destroy(struct retif_scheduler* s);

void retif_scheduler_delete(struct retif_scheduler* s, pid_t ppid);

/**
 * @brief Creates a reservation if possible
 *
 * Creates a reservation if possible, copying parameter from request and then
 * trying to match the parameter with a scheduling plugin. Returns -1 if it
 * is impossible to accept the reservation, 0 if the reservation is accepted
 * partially or 1 if the reservation is accepted.
 *
 * @param s pointer to scheduler data struct
 * @param tp pointer to a struct that contains task params
 * @param ppid process id of the main process of the client
 * @return -1 if refused, 0 if accepted partially, 1 if accepted
 */
int retif_scheduler_task_create(struct retif_scheduler* s, struct retif_params* tp, pid_t ppid);

int retif_scheduler_task_change(struct retif_scheduler* s, struct retif_params* tp, retif_id_t retif_id);

int retif_scheduler_task_attach(struct retif_scheduler* s, retif_id_t retif_id, pid_t pid);

int retif_scheduler_task_detach(struct retif_scheduler* s, retif_id_t retif_id);

int retif_scheduler_task_destroy(struct retif_scheduler* s, retif_id_t retif_id);

void retif_scheduler_dump(struct retif_scheduler* s);

#endif	// RETIF_SCHEDULER_H
