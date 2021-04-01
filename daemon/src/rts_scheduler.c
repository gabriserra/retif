#include <sys/sysinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "logger.h"
#include "rts_scheduler.h"
#include "rts_taskset.h"
#include "rts_task.h"
#include "rts_plugin.h"
#include "rts_config.h"
#include "rts_utils.h"

static int rts_scheduler_test_and_assign(struct rts_scheduler* s, struct rts_task* t) 
{
    int* results = calloc(s->num_of_plugins, sizeof(int));

    for(int i = 0; i < s->num_of_plugins; i++) 
    {
        results[i] = s->plugin[i].rts_plg_task_accept(&(s->plugin[i]), s->taskset, t);
        
        if (results[i] == RTS_OK)
        {
            rts_taskset_add_top(s->taskset, t);
            s->plugin[i].rts_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTS_OK;
        }
    }

    // means no plugin answered with RTS OK

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        if (results[i] == RTS_PARTIAL)
        {
            rts_taskset_add_top(s->taskset, t);
            s->plugin[i].rts_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTS_PARTIAL;
        }
    }

    // means no plugin available
    free(results);
    return RTS_NO;
}

static int rts_scheduler_test_and_modify(struct rts_scheduler* s, struct rts_task* t) 
{
    int* results = calloc(s->num_of_plugins, sizeof(int));

    for(int i = 0; i < s->num_of_plugins; i++) 
    {
        results[i] = s->plugin[i].rts_plg_task_change(&(s->plugin[i]), s->taskset, t);
        
        if (results[i] == RTS_OK)
        {
            s->plugin[i].rts_plg_task_release(&(s->plugin[i]), s->taskset, t);
            s->plugin[i].rts_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTS_OK;
        }
    }

    // means no plugin answered with RTS OK

    for (int i = 0; i < s->num_of_plugins; i++)
    {
        if (results[i] == RTS_PARTIAL)
        {
            s->plugin[i].rts_plg_task_release(&(s->plugin[i]), s->taskset, t);
            s->plugin[i].rts_plg_task_schedule(&(s->plugin[i]), s->taskset, t);

            free(results);
            return RTS_PARTIAL;
        }
    }

    // means no plugin available
    free(results);
    return RTS_NO;
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
int rts_scheduler_init(struct rts_scheduler* s, struct rts_taskset* ts) 
{
    float sys_rt_util;

    if (rts_config_apply() < 0)
    {
        WARN("Unable to apply param configurations. Daemon will continue.\n");
    }
    
    if (rts_config_get_rt_kernel_max_util(&sys_rt_util) < 0)
    {
        WARN("Unable to read rt proc files.\n");
        WARN("Daemon will continue assuming 95%% as max utilization.\n");
        sys_rt_util = 0.95;
    }
    
    s->taskset                  = ts;
    s->last_task_id             = 0;
    s->num_of_cpu               = get_nprocs2();

    return rts_plugins_init(&(s->plugin), &(s->num_of_plugins));
}

/**
 * @internal
 * 
 * Frees scheduler allocated stuff and restore params to default
 * 
 * @endinternal
 */
void rts_scheduler_destroy(struct rts_scheduler* s) 
{
    rts_plugins_destroy(s->plugin, s->num_of_plugins);
    rts_config_restore_rr_kernel_param();
    rts_config_restore_rt_kernel_params();
}

void rts_scheduler_delete(struct rts_scheduler* s, pid_t ppid) 
{   
    struct rts_task* t;
    
    while(1) 
    {
        t = rts_taskset_remove_by_ppid(s->taskset, ppid);
        
        if(t == NULL)
            break;

        s->plugin[t->pluginid].rts_plg_task_release(&(s->plugin[t->pluginid]), s->taskset, t);
        rts_task_destroy(t);
    }
}

/**
 * @internal
 * 
 * Creates a reservation if possible
 * 
 * @endinternal
 */
int rts_scheduler_task_create(struct rts_scheduler* s, struct rts_params* tp, pid_t ppid) 
{
    struct rts_task* t;
    
    rts_task_init(&t, 0, CLK);
    
    t->id           = ++s->last_task_id;
    t->ptid         = ppid;
    t->pluginid     = -1;

    memcpy(&(t->params), tp, sizeof(struct rts_params));
    
    return rts_scheduler_test_and_assign(s, t);
}

int rts_scheduler_task_change(struct rts_scheduler* s, struct rts_params* tp, rts_id_t rts_id) 
{
    struct rts_task* t = rts_taskset_search(s->taskset, rts_id);
    
    if (t == NULL)
        return RTS_ERROR;
    
    return rts_scheduler_test_and_modify(s, t);
}

int rts_scheduler_task_attach(struct rts_scheduler* s, rts_id_t rts_id, pid_t pid) 
{
    struct rts_task* t = rts_taskset_search(s->taskset, rts_id);
    
    if (t == NULL)
        return RTS_ERROR;
    
    t->tid = pid;
    return s->plugin[t->pluginid].rts_plg_task_attach(t);
}

int rts_scheduler_task_detach(struct rts_scheduler* s, rts_id_t rts_id) 
{
    struct rts_task* t = rts_taskset_search(s->taskset, rts_id);

    if (t == NULL)
        return RTS_ERROR;

    return s->plugin[t->pluginid].rts_plg_task_detach(t);
}

int rts_scheduler_task_destroy(struct rts_scheduler* s, rts_id_t rts_id) 
{
    struct rts_task* t;
    
    t = rts_taskset_remove_by_rsvid(s->taskset, rts_id);
    
    if(t == NULL)
        return RTS_ERROR;
    
    s->plugin[t->pluginid].rts_plg_task_release(&(s->plugin[t->pluginid]), s->taskset, t);
    rts_task_destroy(t);
        
    return RTS_OK;
}

void rts_scheduler_dump(struct rts_scheduler* s)
{
    struct rts_task* t;
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
    it = rts_taskset_iterator_init(s->taskset);
    
    while(it != NULL)
    {
        t = rts_taskset_iterator_get_elem(it);
        LOG("Task:\n");
        LOG("-> Plugin %d\n", t->pluginid);
        LOG("-> CPU %ld - PID %d - TID %d - Util: %f \n", t->cpu, t->ptid, t->tid, t->acceptedu);
        it = rts_taskset_iterator_get_next(it);
    }
}