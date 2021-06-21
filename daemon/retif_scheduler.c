#include "retif_scheduler.h"
#include "logger.h"
#include "retif_config.h"
#include "retif_plugin.h"
#include "retif_task.h"
#include "retif_taskset.h"
#include "retif_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <time.h>

static int rtf_scheduler_test_and_assign(struct rtf_scheduler *s,
    struct rtf_task *t)
{
    int *results = calloc(s->num_of_plugins, sizeof(int));

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        results[i] =
            s->plugin[i].rtf_plg_task_accept(&(s->plugin[i]), s->taskset, t);

        if (results[i] == RTF_OK)
        {
            rtf_taskset_add_top(s->taskset, t);
            s->plugin[i].rtf_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTF_OK;
        }
    }

    // means no plugin answered with RTS OK

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        if (results[i] == RTF_PARTIAL)
        {
            rtf_taskset_add_top(s->taskset, t);
            s->plugin[i].rtf_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTF_PARTIAL;
        }
    }

    // means no plugin available
    free(results);
    return RTF_NO;
}

static int rtf_scheduler_test_and_modify(struct rtf_scheduler *s,
    struct rtf_task *t)
{
    int *results = calloc(s->num_of_plugins, sizeof(int));

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        results[i] =
            s->plugin[i].rtf_plg_task_change(&(s->plugin[i]), s->taskset, t);

        if (results[i] == RTF_OK)
        {
            s->plugin[i].rtf_plg_task_release(&(s->plugin[i]), s->taskset, t);
            s->plugin[i].rtf_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTF_OK;
        }
    }

    // means no plugin answered with RTS OK

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        if (results[i] == RTF_PARTIAL)
        {
            s->plugin[i].rtf_plg_task_release(&(s->plugin[i]), s->taskset, t);
            s->plugin[i].rtf_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTF_PARTIAL;
        }
    }

    // means no plugin available
    free(results);
    return RTF_NO;
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
int rtf_scheduler_init(struct rtf_scheduler *s, struct rtf_taskset *ts)
{
    float sys_rt_util;

    if (rtf_config_apply() < 0)
    {
        LOG(WARNING,
            "Unable to apply param configurations. Daemon will continue.\n");
    }

    if (rtf_config_get_rt_kernel_max_util(&sys_rt_util) < 0)
    {
        LOG(WARNING, "Unable to read rt proc files.\n");
        LOG(WARNING,
            "Daemon will continue assuming 95%% as max utilization.\n");
        sys_rt_util = 0.95;
    }

    s->taskset = ts;
    s->last_task_id = 0;
    s->num_of_cpu = get_nprocs2();

    return rtf_plugins_init(&(s->plugin), &(s->num_of_plugins));
}

/**
 * @internal
 *
 * Frees scheduler allocated stuff and restore params to default
 *
 * @endinternal
 */
void rtf_scheduler_destroy(struct rtf_scheduler *s)
{
    rtf_plugins_destroy(s->plugin, s->num_of_plugins);
    rtf_config_restore_rr_kernel_param();
    rtf_config_restore_rt_kernel_params();
}

void rtf_scheduler_delete(struct rtf_scheduler *s, pid_t ppid)
{
    struct rtf_task *t;

    while (1)
    {
        t = rtf_taskset_remove_by_ppid(s->taskset, ppid);

        if (t == NULL)
            break;

        s->plugin[t->pluginid].rtf_plg_task_release(&(s->plugin[t->pluginid]),
            s->taskset, t);
        rtf_task_release(t);
    }
}

/**
 * @internal
 *
 * Creates a reservation if possible
 *
 * @endinternal
 */
int rtf_scheduler_task_create(struct rtf_scheduler *s, struct rtf_params *tp,
    pid_t ppid)
{
    struct rtf_task *t;

    rtf_task_init(&t, 0, CLK);

    t->id = ++s->last_task_id;
    t->ptid = ppid;
    t->pluginid = -1;

    memcpy(&(t->params), tp, sizeof(struct rtf_params));

    return rtf_scheduler_test_and_assign(s, t);
}

int rtf_scheduler_task_change(struct rtf_scheduler *s, struct rtf_params *tp,
    rtf_id_t rtf_id)
{
    struct rtf_task *t = rtf_taskset_search(s->taskset, rtf_id);

    if (t == NULL)
        return RTF_ERROR;

    return rtf_scheduler_test_and_modify(s, t);
}

int rtf_scheduler_task_attach(struct rtf_scheduler *s, rtf_id_t rtf_id,
    pid_t pid)
{
    struct rtf_task *t = rtf_taskset_search(s->taskset, rtf_id);

    if (t == NULL)
        return RTF_ERROR;

    t->tid = pid;
    return s->plugin[t->pluginid].rtf_plg_task_attach(t);
}

int rtf_scheduler_task_detach(struct rtf_scheduler *s, rtf_id_t rtf_id)
{
    struct rtf_task *t = rtf_taskset_search(s->taskset, rtf_id);

    if (t == NULL)
        return RTF_ERROR;

    return s->plugin[t->pluginid].rtf_plg_task_detach(t);
}

int rtf_scheduler_task_destroy(struct rtf_scheduler *s, rtf_id_t rtf_id)
{
    struct rtf_task *t;

    t = rtf_taskset_remove_by_rsvid(s->taskset, rtf_id);

    if (t == NULL)
        return RTF_ERROR;

    s->plugin[t->pluginid].rtf_plg_task_release(&(s->plugin[t->pluginid]),
        s->taskset, t);
    rtf_task_release(t);

    return RTF_OK;
}

void rtf_scheduler_dump(struct rtf_scheduler *s)
{
    struct rtf_task *t;
    iterator_t it;

    LOG(DEBUG, "Number of CPUs: %d\n", s->num_of_cpu);
    LOG(DEBUG, "Number of plugins: %d\n", s->num_of_plugins);

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        LOG(DEBUG, "-> Plugin: %s | %s\n", s->plugin[i].name,
            s->plugin[i].path);

        for (int j = 0; j < s->plugin[i].cputot; j++)
            LOG(DEBUG, "--> CPU %d - Free: %f - Task count: %d\n",
                s->plugin[i].cpulist[j], s->plugin[i].util_free_percpu[j],
                s->plugin[i].task_count_percpu[j]);
    }

    LOG(DEBUG, "Tasks:\n");
    it = rtf_taskset_iterator_init(s->taskset);

    while (it != NULL)
    {
        t = rtf_taskset_iterator_get_elem(it);
        LOG(DEBUG, "Task:\n");
        LOG(DEBUG, "-> Plugin %d\n", t->pluginid);
        LOG(DEBUG, "-> CPU %ld - PID %d - TID %d - Util: %f \n", t->cpu,
            t->ptid, t->tid, t->acceptedu);
        it = rtf_taskset_iterator_get_next(it);
    }
}
