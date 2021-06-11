#ifndef RETIF_SCHEDULER_H
#define RETIF_SCHEDULER_H

#include <sys/types.h>
#include "retif_types.h"
#include "retif_plugin.h"

#define CLK             CLOCK_MONOTONIC

struct rtf_taskset;
struct rtf_task;

struct rtf_scheduler
{
    int                     num_of_cpu;
    int                     num_of_plugins;
    long                    last_task_id;
    struct rtf_taskset*     taskset;
    struct rtf_plugin*      plugin;
};

/**
 * @brief Initializes all data needed for scheduler
 *
 * Initializes all data needed for scheduler (configuration & plugins)
 *
 * @param s pointer to scheduler data struct
 * @param ts pointer to taskset data struct
 */
int rtf_scheduler_init(struct rtf_scheduler* s, struct rtf_taskset* ts);

/**
 * @brief Deallocate scheduler stuff
 *
 * Frees scheduler allocated stuff and restore params to default
 *
 * @param s pointer to scheduler data struct
 */
void rtf_scheduler_destroy(struct rtf_scheduler* s);

void rtf_scheduler_delete(struct rtf_scheduler* s, pid_t ppid);

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
int rtf_scheduler_task_create(struct rtf_scheduler* s, struct rtf_params* tp, pid_t ppid);

int rtf_scheduler_task_change(struct rtf_scheduler* s, struct rtf_params* tp, rtf_id_t rtf_id);

int rtf_scheduler_task_attach(struct rtf_scheduler* s, rtf_id_t rtf_id, pid_t pid);

int rtf_scheduler_task_detach(struct rtf_scheduler* s, rtf_id_t rtf_id);

int rtf_scheduler_task_destroy(struct rtf_scheduler* s, rtf_id_t rtf_id);

void rtf_scheduler_dump(struct rtf_scheduler* s);

#endif	// RETIF_SCHEDULER_H
