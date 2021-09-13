#include "terminal.h"
#include <retif.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void printCPUs(unsigned int nplug, unsigned int ncpu)
{
    struct rtf_cpu_info data;

    for (int i = 0; i < ncpu; i++)
    {
        if (rtf_plugin_cpu_info(nplug, i, &data) == RTF_ERROR)
        {
            printf("#%d \t Unable to get data\n", i);
            continue;
        }
        else
        {
            printf("\t CPU: #%d \t Free util : %f \t Num of tasks : %d\n",
                data.cpunum, data.freeu, data.ntask);
        }
    }
}

void printPlugins()
{
    int nplug;
    struct rtf_plugin_info data;

    nplug = rtf_plugins_info();

    if (nplug == RTF_ERROR)
        printerr("> Unable to get plugins info\n");

    yellow();
    printf("> Plugin registered\n");
    printf("> Number of plugin: %d\n", nplug);
    reset();

    for (int i = 0; i < nplug; i++)
    {
        if (rtf_plugin_info(i, &data) == RTF_ERROR)
        {
            printf("#%d \t Unable to get data\n", i);
        }
        else
        {
            printf("#%d \t Name: %s \t CPUs: %d\n", i, data.name, data.cputot);
            printCPUs(i, data.cputot);
        }
    }
}

void printConnections()
{
    int nconn;
    struct rtf_client_info data;

    nconn = rtf_connections_info();

    if (nconn == RTF_ERROR)
        printerr("> Unable to get plugins info\n");

    yellow();
    printf("> Connected clients\n");
    printf("> Number of clients: %d\n", nconn);
    reset();

    for (int desc = 0, count = 0; count < nconn; desc++)
    {
        if (rtf_connection_info(desc, &data) == RTF_ERROR)
        {
            continue;
        }
        else
        {
            printf("#%d \t PID: %d \t State: %s\n", desc, data.pid,
                stringFromState(data.state));
            count++;
        }
    }
}

void printTasks()
{
    int ntask;
    struct rtf_task_info data;

    ntask = rtf_tasks_info();

    if (ntask == RTF_ERROR)
        printerr("> Unable to get tasks info\n");

    yellow();
    printf("> Task instances\n");
    printf("> Number of tasks: %d\n", ntask);
    reset();

    for (int i = 1; i <= ntask; i++)
    {
        if (rtf_task_info(i, &data) == RTF_ERROR)
            printf("#%d \t Unable to get data\n", i);
        else
            printf("#%d \t TID : %d \t PPID : %d \t Priority : %d \t Period : "
                   "%d \t Util : %f \t Plugin : %s \n",
                i, data.tid, data.ppid, data.priority, data.period, data.util,
                stringFromName(data.pluginid));
    }
}

int main()
{
    // connect with daemon
    if (rtf_connect() != RTF_OK)
        printerr("> Unable to connect with RTF\n");

    while (1)
    {
        clear();
        printConnections();
        printPlugins();
        printTasks();
        spinner();
    }

    return 0;
}
