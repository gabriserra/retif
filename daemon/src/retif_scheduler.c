#include <sys/sysinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "logger.h"
#include "retif_scheduler.h"
#include "retif_taskset.h"
#include "retif_task.h"
#include "retif_plugin.h"
#include "retif_config.h"
#include "retif_utils.h"

static int retif_scheduler_test_and_assign(struct retif_scheduler* s, struct retif_task* t)
{
    int* results = calloc(s->num_of_plugins, sizeof(int));

    for(int i = 0; i < s->num_of_plugins; i++)
    {
        results[i] = s->plugin[i].retif_plg_task_accept(&(s->plugin[i]), s->taskset, t);

        if (results[i] == retif_OK)
        {
            retif_taskset_add_top(s->taskset, t);
            s->plugin[i].retif_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return retif_OK;
        }
    }

    // means no plugin answered with RTS OK

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        if (results[i] == retif_PARTIAL)
        {
            retif_taskset_add_top(s->taskset, t);
            s->plugin[i].retif_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return retif_PARTIAL;
        }
    }

    // means no plugin available
    free(results);
    return retif_NO;
}

static int retif_scheduler_test_and_modify(struct retif_scheduler* s, struct retif_task* t)
{
    int* results = calloc(s->num_of_plugins, sizeof(int));

    for(int i = 0; i < s->num_of_plugins; i++)
    {
        results[i] = s->plugin[i].retif_plg_task_change(&(s->plugin[i]), s->taskset, t);

        if (results[i] == retif_OK)
        {
            s->plugin[i].retif_plg_task_release(&(s->plugin[i]), s->taskset, t);
            s->plugin[i].retif_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return retif_OK;
        }
    }

    // means no plugin answered with RTS OK

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        if (results[i] == retif_PARTIAL)
        {
            s->plugin[i].retif_plg_task_release(&(s->plugin[i]), s->taskset, t);
            s->plugin[i].retif_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return retif_PARTIAL;
        }
    }

    // means no plugin available
    free(results);
    return retif_NO;
}

// -----------------------------------------------------------------------------
// PUBLIC METHODS
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * Initializes all data needed for scheduler (configuration & plugins)
 *
 * @endinternal
 */
int retif_scheduler_init(struct retif_scheduler* s, struct retif_taskset* ts)
{
    float sys_rt_util;

    if (retif_config_apply() < 0)
    {
        WARN("Unable to apply param configurations. Daemon will continue.\n");
    }

    if (retif_config_get_rt_kernel_max_util(&sys_rt_util) < 0)
    {
        WARN("Unable to read rt proc files.\n");
        WARN("Daemon will continue assuming 95%% as max utilization.\n");
        sys_rt_util = 0.95;
    }

    s->taskset                  = ts;
    s->last_task_id             = 0;
    s->num_of_cpu               = get_nprocs2();

    return retif_plugins_init(&(s->plugin), &(s->num_of_plugins));
}

/**
 * @internal
 *
 * Frees scheduler allocated stuff and restore params to default
 *
 * @endinternal
 */
void retif_scheduler_destroy(struct retif_scheduler* s)
{
    retif_plugins_destroy(s->plugin, s->num_of_plugins);
    retif_config_restore_rr_kernel_param();
    retif_config_restore_rt_kernel_params();
}

void retif_scheduler_delete(struct retif_scheduler* s, pid_t ppid)
{
    struct retif_task* t;

    while(1)
    {
        t = retif_taskset_remove_by_ppid(s->taskset, ppid);

        if(t == NULL)
            break;

        s->plugin[t->pluginid].retif_plg_task_release(&(s->plugin[t->pluginid]), s->taskset, t);
        retif_task_destroy(t);
    }
}

/**
 * @internal
 *
 * Creates a reservation if possible
 *
 * @endinternal
 */
int retif_scheduler_task_create(struct retif_scheduler* s, struct retif_params* tp, pid_t ppid)
{
    struct retif_task* t;

    retif_task_init(&t, 0, CLK);

    t->id           = ++s->last_task_id;
    t->ptid         = ppid;
    t->pluginid     = -1;

    memcpy(&(t->params), tp, sizeof(struct retif_params));

    return retif_scheduler_test_and_assign(s, t);
}

int retif_scheduler_task_change(struct retif_scheduler* s, struct retif_params* tp, retif_id_t retif_id)
{
    struct retif_task* t = retif_taskset_search(s->taskset, retif_id);

    if (t == NULL)
        return retif_ERROR;

    return retif_scheduler_test_and_modify(s, t);
}

int retif_scheduler_task_attach(struct retif_scheduler* s, retif_id_t retif_id, pid_t pid)
{
    struct retif_task* t = retif_taskset_search(s->taskset, retif_id);

    if (t == NULL)
        return retif_ERROR;

    t->tid = pid;
    return s->plugin[t->pluginid].retif_plg_task_attach(t);
}

int retif_scheduler_task_detach(struct retif_scheduler* s, retif_id_t retif_id)
{
    struct retif_task* t = retif_taskset_search(s->taskset, retif_id);

    if (t == NULL)
        return retif_ERROR;

    return s->plugin[t->pluginid].retif_plg_task_detach(t);
}

int retif_scheduler_task_destroy(struct retif_scheduler* s, retif_id_t retif_id)
{
    struct retif_task* t;

    t = retif_taskset_remove_by_rsvid(s->taskset, retif_id);

    if(t == NULL)
        return retif_ERROR;

    s->plugin[t->pluginid].retif_plg_task_release(&(s->plugin[t->pluginid]), s->taskset, t);
    retif_task_destroy(t);

    return retif_OK;
}

void retif_scheduler_dump(struct retif_scheduler* s)
{
    struct retif_task* t;
    iterator_t it;

    LOG("Number of CPUs: %d\n", s->num_of_cpu);
    LOG("Number of plugins: %d\n", s->num_of_plugins);

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        LOG("-> Plugin: %s | %s\n", s->plugin[i].name, s->plugin[i].path);

        for (int j = 0; j < s->plugin[i].cputot; j++)
            LOG("--> CPU %d - Free: %f - Task count: %d\n",
                s->plugin[i].cpulist[j],
                s->plugin[i].util_free_percpu[j],
                s->plugin[i].task_count_percpu[j]);
    }

    LOG("Tasks:\n");
    it = retif_taskset_iterator_init(s->taskset);

    while(it != NULL)
    {
        t = retif_taskset_iterator_get_elem(it);
        LOG("Task:\n");
        LOG("-> Plugin %d\n", t->pluginid);
        LOG("-> CPU %ld - PID %d - TID %d - Util: %f \n", t->cpu, t->ptid, t->tid, t->acceptedu);
        it = retif_taskset_iterator_get_next(it);
    }
}
