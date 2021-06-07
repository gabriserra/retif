#ifndef retif_PLUGIN_H
#define retif_PLUGIN_H

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

#define retif_API_INIT        "retif_plg_task_init"
#define retif_API_ACCEPT      "retif_plg_task_accept"
#define retif_API_CHANGE      "retif_plg_task_change"
#define retif_API_RELEASE     "retif_plg_task_release"
#define retif_API_SCHEDULE    "retif_plg_task_schedule"
#define retif_API_ATTACH      "retif_plg_task_attach"
#define retif_API_DETACH      "retif_plg_task_detach"

// forward declarations, see retif_task.c and retif_taskset.c
struct retif_task;
struct retif_taskset;
struct retif_plugin;

/**
 * @brief Used by plugin to initializes itself
 */
typedef int (*retif_plg_task_init_pfun)(struct retif_plugin*);

/**
 * @brief Used by plugin to perform a new task admission test
 */
typedef int (*retif_plg_task_accept_pfun)(struct retif_plugin*, struct retif_taskset*, struct retif_task*);

/**
 * @brief Used by plugin to perform a new admission test when task modifies parameters
 */
typedef int (*retif_plg_task_change_pfun)(struct retif_plugin*, struct retif_taskset*, struct retif_task*);

/**
 * @brief Used by plugin to perform a release of previous accepted task
 */
typedef int (*retif_plg_task_release_pfun)(struct retif_plugin*, struct retif_taskset*, struct retif_task*);

/**
 * @brief Used by plugin to set the task as accepted
 */
typedef void (*retif_plg_task_schedule_pfun)(struct retif_plugin*, struct retif_taskset*, struct retif_task*);

/**
 * @brief Used by plugin to set rt scheduler for a task
 */
typedef int (*retif_plg_task_attach_pfun)(struct retif_task*);

/**
 * @brief Used by plugin to reset scheduler (other) for a task
 */
typedef int (*retif_plg_task_detach_pfun)(struct retif_task*);

/**
 * @brief Plugin data structure, common to all plugins
 */
struct retif_plugin
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
    struct retif_taskset*         tasks;
    retif_plg_task_init_pfun      retif_plg_task_init;
    retif_plg_task_accept_pfun    retif_plg_task_accept;
    retif_plg_task_change_pfun    retif_plg_task_change;
    retif_plg_task_release_pfun   retif_plg_task_release;
    retif_plg_task_schedule_pfun  retif_plg_task_schedule;
    retif_plg_task_attach_pfun    retif_plg_task_attach;
    retif_plg_task_detach_pfun    retif_plg_task_detach;
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
int retif_plugins_init(struct retif_plugin** plgs, int* num_of_plugins);

/**
 * @brief Tear down plugin data structure
 *
 * Closes dynamic library opened in init phase and free allocated memory.
 * Must be used when tearing down daemon.
 *
 * @param plgs pointer to plugin structure that will be freed
 * @param plugin_num number of plugins that need to be freed
 */
void retif_plugins_destroy(struct retif_plugin* plgs, int plugin_num);

#endif /* retif_PLUGIN_H */