#ifndef RTS_PLUGIN_H
#define RTS_PLUGIN_H

// -----------------------------------------------------------------------------
// PLUGINS MACROS / CONSTANTS
// -----------------------------------------------------------------------------

#define PLUGIN_MAX_NAME         10
#define PLUGIN_MAX_PATH         40
#define PLUGIN_CFG              "/usr/share/rtsd/schedconfig.cfg"
#define PLUGIN_PREFIX           "/usr/share/rtsd/plugins/sched_"

// -----------------------------------------------------------------------------
// SYMBOLS TO LOAD FROM LIBRARIES
// -----------------------------------------------------------------------------

#define RTS_API_INIT        "rts_plg_task_init"
#define RTS_API_ACCEPT      "rts_plg_task_accept"
#define RTS_API_CHANGE      "rts_plg_task_change"
#define RTS_API_RELEASE     "rts_plg_task_release"
#define RTS_API_SCHEDULE    "rts_plg_task_schedule"
#define RTS_API_ATTACH      "rts_plg_task_attach"
#define RTS_API_DETACH      "rts_plg_task_detach"

// forward declarations, see rts_task.c and rts_taskset.c
struct rts_task;
struct rts_taskset;
struct rts_plugin;

/**
 * @brief Used by plugin to initializes itself
 */
typedef int (*rts_plg_task_init_pfun)(struct rts_plugin*);

/**
 * @brief Used by plugin to perform a new task admission test
 */
typedef int (*rts_plg_task_accept_pfun)(struct rts_plugin*, struct rts_taskset*, struct rts_task*);

/**
 * @brief Used by plugin to perform a new admission test when task modifies parameters
 */
typedef int (*rts_plg_task_change_pfun)(struct rts_plugin*, struct rts_taskset*, struct rts_task*);

/**
 * @brief Used by plugin to perform a release of previous accepted task
 */
typedef int (*rts_plg_task_release_pfun)(struct rts_plugin*, struct rts_taskset*, struct rts_task*);

/**
 * @brief Used by plugin to set the task as accepted
 */
typedef void (*rts_plg_task_schedule_pfun)(struct rts_plugin*, struct rts_taskset*, struct rts_task*);

/**
 * @brief Used by plugin to set rt scheduler for a task
 */
typedef int (*rts_plg_task_attach_pfun)(struct rts_task*);

/**
 * @brief Used by plugin to reset scheduler (other) for a task
 */
typedef int (*rts_plg_task_detach_pfun)(struct rts_task*);

/**
 * @brief Plugin data structure, common to all plugins
 */
struct rts_plugin 
{
    int                         id;
    char                        name[PLUGIN_MAX_NAME];
    char                        path[PLUGIN_MAX_PATH];
    void*                       dl_ptr;    
    int                         prio_min;
    int                         prio_max;
    int*                        cpulist;
    int                         cpunum;
    int                         cputot;
    float*                      util_free_percpu;
    int*                        task_count_percpu;
    struct rts_taskset*         tasks;
    rts_plg_task_init_pfun      rts_plg_task_init;
    rts_plg_task_accept_pfun    rts_plg_task_accept;
    rts_plg_task_change_pfun    rts_plg_task_change;
    rts_plg_task_release_pfun   rts_plg_task_release;
    rts_plg_task_schedule_pfun  rts_plg_task_schedule;
    rts_plg_task_attach_pfun    rts_plg_task_attach;
    rts_plg_task_detach_pfun    rts_plg_task_detach;
};

// -----------------------------------------------------------------------------
// PUBLIC METHODS
// -----------------------------------------------------------------------------

/**
 * @brief Initializes plugin data structure
 * 
 * Initializes plugins data structure reading settings from config file and
 * loading dynamically symbols from shared lib. Return -1 if unable to open
 * config file or options specified are not valid, 0 in case of success.
 * 
 * @param plgs pointer to plugin structure that will be initialized
 * @param num_of_plugins number of plugins found
 * @return -1 in case of error, 0 in case of success
 */
int rts_plugins_init(struct rts_plugin** plgs, int* num_of_plugins);

/**
 * @brief Tear down plugin data structure
 * 
 * Closes dynamic library opened in init phase and free allocated memory.
 * Must be used when tearing down daemon.
 * 
 * @param plgs pointer to plugin structure that will be freed
 * @param plugin_num number of plugins that need to be freed
 */
void rts_plugins_destroy(struct rts_plugin* plgs, int plugin_num);

#endif /* RTS_PLUGIN_H */