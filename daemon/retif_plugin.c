#include "retif_plugin.h"
#include "logger.h"
#include "retif_config.h"
#include "retif_daemon.h"
#include "retif_taskset.h"
#include "retif_utils.h"
#include "vector.h"
#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

// -----------------------------------------------------------------------------
// PUBLIC METHODS
// -----------------------------------------------------------------------------

char *fullpath(const char *path, const char *relpath)
{
    char *fpath = calloc(strlen(path) + strlen(relpath) + 1, sizeof(char));
    if (fpath == NULL)
        return NULL;
    strcat(fpath, path);
    strcat(fpath, relpath);
    return fpath;
}

int open_plugin_dll(const char *path, const char *relpath,
    struct rtf_plugin *plg)
{
    char *fpath = fullpath(path, relpath);
    if (access(fpath, F_OK) != 0)
    {
        free(fpath);
        // File not found
        return -2;
    }

    void *dl_ptr = dlopen(fpath, RTLD_NOW);
    free(fpath);
    if (dl_ptr == NULL)
    {
        // Error opening the dll
        return -1;
    }

    plg->dl_ptr = dl_ptr;
    plg->rtf_plg_task_init = dlsym(dl_ptr, RTF_API_INIT);
    plg->rtf_plg_task_accept = dlsym(dl_ptr, RTF_API_ACCEPT);
    plg->rtf_plg_task_change = dlsym(dl_ptr, RTF_API_CHANGE);
    plg->rtf_plg_task_release = dlsym(dl_ptr, RTF_API_RELEASE);
    plg->rtf_plg_task_schedule = dlsym(dl_ptr, RTF_API_SCHEDULE);
    plg->rtf_plg_task_attach = dlsym(dl_ptr, RTF_API_ATTACH);
    plg->rtf_plg_task_detach = dlsym(dl_ptr, RTF_API_DETACH);

    // FIXME: Check all symbols not NULL
    return 0;
}

int find_and_open_plugin(struct rtf_plugin *plg)
{
    if (strlen(plg->path) < 1)
    {
        LOG(ERR, "Unable to open %s plugin: %s.\n", plg->name, "Empty path");
        return -1;
    }

    int res;
    if (plg->path[0] == '/')
    {
        res = open_plugin_dll("", plg->path, plg);
        switch (res)
        {
        case -2:
            LOG(ERR, "Unable to open %s plugin from path %s: %s.\n", plg->name,
                plg->path, "File not found");
            return -1;
        case -1:
            LOG(ERR, "Unable to open %s plugin from path %s: %s.\n", plg->name,
                plg->path, dlerror());
            return -1;
        default:
            return 0;
        }
    }

    char *curpath = calloc(strlen(conf_file_path) + 1, sizeof(char));
    strcpy(curpath, conf_file_path);
    dirname(curpath);
    strcat(curpath, "/");

    // Current working directory
    res = open_plugin_dll(curpath, plg->path, plg);
    switch (res)
    {
    case 0:

        res = 0;
        goto end;
    case -1:
        LOG(ERR, "Unable to open %s plugin from path %s: %s.\n", plg->name,
            fullpath(curpath, plg->path), dlerror());
        res = -1;
        goto end;
    }

    // Default directory
    res = open_plugin_dll(PLUGIN_DEFAULT_INSTALLPATH, plg->path, plg);
    switch (res)
    {
    case 0:
        res = 0;
        goto end;
    case -1:
        LOG(ERR, "Unable to open %s plugin from path %s: %s.\n", plg->name,
            fullpath(PLUGIN_DEFAULT_INSTALLPATH, plg->path), dlerror());
        res = -1;
        goto end;
    }

    LOG(ERR, "Unable to locate %s plugin from %s.\n", plg->name, plg->path);
    res = -1;
    goto end;

end:
    free(curpath);
    return res;
}

/**
 * @internal
 *
 * Initializes plugins data structure reading settings from config file and
 * loading dynamically symbols from shared lib. Return -1 if unable to open
 * config file or options specified are not valid, 0 in case of success.
 *
 * @endinternal
 */
int rtf_plugins_init(vector_conf_plugin_t *confs, struct rtf_plugin **out_plgs,
    int *num_of_plugins)
{
    size_t i, j;

    (*num_of_plugins) = confs->size;
    (*out_plgs) = calloc((*num_of_plugins), sizeof(struct rtf_plugin));

    struct rtf_plugin *plgs = *out_plgs;

    size_t num_cpu = get_nprocs2();

    for (i = 0; i < confs->size; ++i)
    {
        plgs[i].id = i;
        plgs[i].cputot = confs->data[i].cores.size;
        plgs[i].prio_min = confs->data[i].priority_min;
        plgs[i].prio_max = confs->data[i].priority_max;

        plgs[i].cpulist = calloc(plgs[i].cputot, sizeof(int));
        plgs[i].util_free_percpu = calloc(plgs[i].cputot, sizeof(int));
        plgs[i].task_count_percpu = calloc(plgs[i].cputot, sizeof(int));
        plgs[i].tasks = calloc(plgs[i].cputot, sizeof(struct rtf_taskset));
        plgs[i].name = calloc(strlen(confs->data[i].name) + 1, sizeof(char));
        plgs[i].path =
            calloc(strlen(confs->data[i].plugin_path) + 1, sizeof(char));

        // FIXME: use max_util from conf
        for (j = 0; j < plgs[i].cputot; j++)
            plgs[i].util_free_percpu[j] = 1;

        for (j = 0; j < plgs[i].cputot; j++)
            rtf_taskset_init(&plgs[i].tasks[j]);

        strcpy(plgs[i].name, confs->data[i].name);

        memmove(plgs[i].cpulist, confs->data[i].cores.data,
            sizeof(int) * plgs[i].cputot);

        strcpy(plgs[i].path, confs->data[i].plugin_path);
        if (find_and_open_plugin(&plgs[i]) != 0)
            return -1;
    } // endfor

    return 0;
}

/**
 * @internal
 *
 * Closes dynamic library opened in init phase and free allocated memory.
 * Must be used when tearing down daemon
 *
 * @endinternal
 */
void rtf_plugins_destroy(struct rtf_plugin *plgs, int plugin_num)
{
    for (int i = 0; i < plugin_num; i++)
    {
        // TODO: missing frees
        free(plgs[i].util_free_percpu);
        free(plgs[i].cpulist);
        free(plgs[i].task_count_percpu);
        dlclose(plgs[i].dl_ptr);
    }
}
