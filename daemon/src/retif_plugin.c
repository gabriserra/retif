#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "logger.h"
#include "retif_utils.h"
#include "retif_plugin.h"
#include "retif_taskset.h"

// -----------------------------------------------------------------------------
// PRIVATE INTERNAL METHODS
// -----------------------------------------------------------------------------

static void read_plg_name(char* line, char* name)
{
    sscanf(line, "%s", name);
}

static void read_plg_priopool(char* line, int* prio_min, int* prio_max)
{
    sscanf(line, "%d%*[-/]%d", prio_min, prio_max);
}

static void read_plg_cpus(char* line, int* cputotnum, int* cpulist)
{
    int cnum;
    int cmax;
    char* token;
    char* init;
    char* end;

    cnum = 0;
    cmax = get_nprocs2();

    init = strtok(line, "-");
    end = strtok(NULL, "-");

    // cpu expressed in num-num format
    if (init != NULL && end != NULL)
    {
        cnum = atoi(end) - atoi(init) + 1;

        if (cnum >= cmax)
        {
            ERR("Your configuration include a greater number of CPU than present.\n");
            return;
        }

        for (int i = 0; i <= cnum; i++)
            cpulist[i] = atoi(init) + i;

        *cputotnum = cnum;

        return;
    }

    // cpu expressed in num,num,num format
    token = strtok(line, ",");

    while (token != NULL)
    {
        if (cnum >= cmax)
        {
            ERR("Your configuration include a greater number of CPU than present.\n");
            break;
        }

        cpulist[cnum] = atoi(token);
        token = strtok(NULL, ",");
        cnum++;
    }

    *cputotnum = cnum;
}

/**
 * @internal
 *
 * Reads plugins names and priority pools from schedcfg file and fills
 * given plugin structure
 *
 * @endinternal
 */
static void read_conf(FILE* f, struct retif_plugin* plg, int num_of_plugin)
{
    int i, j;
    int num_cpu;
    char buffer[CFG_COLUMN_MAX];

    num_cpu = get_nprocs2();

    for (i = 0; i < num_of_plugin; i++)
    {
        // TODO: Find a way to limit max str
        // TODO: Check plugin priory windows are consistent with order

        plg[i].cpulist              = calloc(num_cpu, sizeof(int));
        plg[i].util_free_percpu     = calloc(num_cpu, sizeof(int));
        plg[i].task_count_percpu    = calloc(num_cpu, sizeof(int));
        plg[i].tasks                = calloc(num_cpu, sizeof(struct retif_taskset));

        // set free util to 1
        for (j = 0; j < num_cpu; j++)
            plg[i].util_free_percpu[j] = 1;

        // initialize tasksets per cpu
        for (j = 0; j < num_cpu; j++)
            retif_taskset_init(&plg[i].tasks[j]);

        safe_file_read(f, "%s", 1, buffer);
        read_plg_name(buffer, plg[i].name);
        safe_file_read(f, "%s", 1, buffer);
        read_plg_priopool(buffer, &(plg[i].prio_min), &(plg[i].prio_max));
        safe_file_read(f, "%s", 1, buffer);
        read_plg_cpus(buffer, &(plg[i].cputot), plg[i].cpulist);
    }
}

/**
 * @internal
 *
 * Loads dinamically all symbols needed by a plugin
 *
 * @endinternal
 */
static void load_symbols(struct retif_plugin* plg, unsigned int index, void* dl_ptr)
{
    int cpunum = get_nprocs2();

    plg[index].id                       = index;
    plg[index].cpunum                   = cpunum;
    plg[index].dl_ptr                   = dl_ptr;
    plg[index].retif_plg_task_init        = dlsym(dl_ptr, retif_API_INIT);
    plg[index].retif_plg_task_accept      = dlsym(dl_ptr, retif_API_ACCEPT);
    plg[index].retif_plg_task_change      = dlsym(dl_ptr, retif_API_CHANGE);
    plg[index].retif_plg_task_release     = dlsym(dl_ptr, retif_API_RELEASE);
    plg[index].retif_plg_task_schedule    = dlsym(dl_ptr, retif_API_SCHEDULE);
    plg[index].retif_plg_task_attach      = dlsym(dl_ptr, retif_API_ATTACH);
    plg[index].retif_plg_task_detach      = dlsym(dl_ptr, retif_API_DETACH);
}

/**
 * @internal
 *
 * Takes plugin structure as input and loads all static libraries needed
 *
 * @endinternal
 */
static int load_libraries(struct retif_plugin* plg, int num_of_plugin)
{
    void* dl_ptr;

    for(int i = 0; i < num_of_plugin; i++)
    {
        strcpy(plg[i].path, PLUGIN_PREFIX);
        strcat(plg[i].path, plg[i].name);
        strcat(plg[i].path, ".so");

        dl_ptr = dlopen(plg[i].path, RTLD_NOW);

        if (dl_ptr == NULL)
        {
            ERR("Unable to open %s plugin. %s\n", plg[i].path, strerror(errno));
            return -1;
        }

        load_symbols(plg, i, dl_ptr);
    }

    return 0;
}

// -----------------------------------------------------------------------------
// PUBLIC METHODS
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * Initializes plugins data structure reading settings from config file and
 * loading dynamically symbols from shared lib. Return -1 if unable to open
 * config file or options specified are not valid, 0 in case of success.
 *
 * @endinternal
 */
int retif_plugins_init(struct retif_plugin** plgs, int* num_of_plugins)
{
    FILE* f;
    int num_plugin;

    f = fopen(PLUGIN_CFG, "r");

    if(f == NULL)
    {
        ERR("Unable to open cfg file. %s. Are plugins installed?\n", strerror(errno));
        return -1;
    }

    go_to_settings_head(f); // skip kernel param settings
    go_to_settings_head(f);

    num_plugin  = count_num_of_settings(f);

    (*num_of_plugins)           = num_plugin;
    (*plgs)                     = calloc(num_plugin, sizeof(struct retif_plugin));

    read_conf(f, *plgs, num_plugin);

    if (load_libraries(*plgs, num_plugin) < 0)
        return -1;

    fclose(f);

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
void retif_plugins_destroy(struct retif_plugin* plgs, int plugin_num)
{
    for(int i = 0; i < plugin_num; i++)
    {
        free(plgs[i].util_free_percpu);
        free(plgs[i].cpulist);
        free(plgs[i].task_count_percpu);
        dlclose(plgs[i].dl_ptr);
    }
}
