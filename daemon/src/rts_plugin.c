#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "logger.h"
#include "rts_utils.h"
#include "rts_plugin.h"

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

    cnum = 0;
    cmax = get_nprocs();
    token = strtok(line, ",");

    while (token != NULL)
    {
        if (cnum >= cmax)
        {
            WARN("Your configuration include a greater number of CPU than present.\n");
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
static void read_conf(FILE* f, struct rts_plugin* plg, int num_of_plugin) 
{
    int i, j;
    int num_cpu;
    char buffer[CFG_COLUMN_MAX];

    num_cpu = get_nprocs();

    for (i = 0; i < num_of_plugin; i++)
    {
        // TODO: Find a way to limit max str
        // TODO: Check plugin priory windows are consistent with order

        plg[i].cpulist            = calloc(num_cpu, sizeof(int));
        plg[i].util_free_percpu   = calloc(num_cpu, sizeof(int));
        plg[i].task_count_percpu  = calloc(num_cpu, sizeof(int));

        // set free util to 1
        for (j = 0; j < num_cpu; j++)
            plg[i].util_free_percpu[j] = 1;

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
static void load_symbols(struct rts_plugin* plg, unsigned int index, void* dl_ptr)
{
    int cpunum = get_nprocs();

    plg[index].id                       = index;
    plg[index].cpunum                   = cpunum;
    plg[index].dl_ptr                   = dl_ptr;
    plg[index].rts_plg_task_init        = dlsym(dl_ptr, RTS_API_INIT);
    plg[index].rts_plg_task_accept      = dlsym(dl_ptr, RTS_API_ACCEPT);
    plg[index].rts_plg_task_change      = dlsym(dl_ptr, RTS_API_CHANGE);
    plg[index].rts_plg_task_release     = dlsym(dl_ptr, RTS_API_RELEASE);
    plg[index].rts_plg_task_schedule    = dlsym(dl_ptr, RTS_API_SCHEDULE);
    plg[index].rts_plg_task_attach      = dlsym(dl_ptr, RTS_API_ATTACH);
    plg[index].rts_plg_task_detach      = dlsym(dl_ptr, RTS_API_DETACH);
}

/**
 * @internal
 * 
 * Takes plugin structure as input and loads all static libraries needed
 * 
 * @endinternal
 */
static int load_libraries(struct rts_plugin* plg, int num_of_plugin) 
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
int rts_plugins_init(struct rts_plugin** plgs, int* num_of_plugins) 
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
    (*plgs)                     = calloc(num_plugin, sizeof(struct rts_plugin));
        
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
void rts_plugins_destroy(struct rts_plugin* plgs, int plugin_num)
{    
    for(int i = 0; i < plugin_num; i++) 
    {
        free(plgs[i].util_free_percpu);
        free(plgs[i].cpulist);
        free(plgs[i].task_count_percpu);
        dlclose(plgs[i].dl_ptr);
    }
}

