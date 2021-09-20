#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * A list of functions to change ANSI terminal colors or clear it
 */

void red()
{
    printf("\033[1;31m");
}

void yellow()
{
    printf("\033[1;33m");
}

void green()
{
    printf("\033[0;32m");
}

void reset()
{
    printf("\033[0m");
}

void clear()
{
    printf("\e[1;1H\e[2J");
}

void printerr(char *msg)
{
    red();
    printf("%s", msg);
    reset();
    exit(-1);
}

void spinner()
{
    int lastc = 0;
    const char spin[] = {'|', '/', '-', '|', '\\', '-'};

    green();
    printf("Monitoring... |%c|", spin[0]);

    for (int i = 0; i < 10; i++)
    {
        usleep(100000);
        printf("\b\b\b|%c|", spin[lastc]);
        fflush(stdout);
        lastc = ++lastc % 6;
    }

    reset();
}

/**
 * Utility to print out enum strings instead of int values
 */

const char *CLIENT_STATE_STRINGS[] = {
    "EMPTY",
    "CONNECTED",
    "DISCONNECTED",
    "ERROR",
};

const char *PLUGIN_NAME_STRINGS[] = {
    "EDF",
    "RM",
    "RR",
    "FP",
};

const char *stringFromState(enum CLIENT_STATE s)
{
    return CLIENT_STATE_STRINGS[s];
}

const char *stringFromName(enum PLUGIN_NAME s)
{
    return PLUGIN_NAME_STRINGS[s];
}
