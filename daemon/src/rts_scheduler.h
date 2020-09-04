#ifndef RTS_SCHEDULER_H
#define RTS_SCHEDULER_H

#include <sys/types.h>
#include "rts_types.h"
#include "rts_plugin.h"

#define CLK             CLOCK_MONOTONIC

struct rts_taskset;
struct rts_task;

struct rts_scheduler 
{    
    int                     num_of_cpu;
    int                     num_of_plugins;
    long                    last_task_id;
    struct rts_taskset*     taskset;
    struct rts_plugin*      plugin;
};

/**
 * @brief Initializes all data needed for scheduler
 * 
 * Initializes all data needed for scheduler (configuration & plugins)
 * 
 * @param s pointer to scheduler data struct
 * @param ts pointer to taskset data struct
 */
int rts_scheduler_init(struct rts_scheduler* s, struct rts_taskset* ts);

/**
 * @brief Deallocate scheduler stuff
 * 
 * Frees scheduler allocated stuff and restore params to default
 * 
 * @param s pointer to scheduler data struct
 */
void rts_scheduler_destroy(struct rts_scheduler* s);

void rts_scheduler_delete(struct rts_scheduler* s, pid_t ppid);

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
int rts_scheduler_task_create(struct rts_scheduler* s, struct rts_params* tp, pid_t ppid);

int rts_scheduler_task_change(struct rts_scheduler* s, struct rts_params* tp, rts_id_t rts_id);

int rts_scheduler_task_attach(struct rts_scheduler* s, rts_id_t rts_id, pid_t pid);

int rts_scheduler_task_detach(struct rts_scheduler* s, rts_id_t rts_id);

int rts_scheduler_task_destroy(struct rts_scheduler* s, rts_id_t rts_id);

int rts_scheduler_get_euid(pid_t pid); 

#endif	// RTS_SCHEDULER_H

