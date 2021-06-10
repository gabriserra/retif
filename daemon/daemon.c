/**
 * @file daemon.c
 * @author Gabriele Serra
 * @date 05 Jan 2020
 * @brief Contains daemon entry & exit points
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "logger.h"
#include "retif_daemon.h"

struct retif_daemon data;

/**
 * @brief When daemon is signaled with a SIGINT, tear down it
 */
void term()
{
    INFO("\nReTiF daemon was interrupted. It will destroy data and stop.\n");
    retif_daemon_destroy(&data);
    exit(EXIT_SUCCESS);
}

/**
 * @brief When daemon is signaled with a SIGTTOU, dump debug info
 */
void output()
{
    INFO("\n--------------------------\n");
    INFO("ReTiF daemon info dump\n");
    INFO("--------------------------\n");
    retif_daemon_dump(&data);
}

/**
 * @brief Main daemon routine, initializes data and starts loop
 */
int main(int argc, char* argv[])
{
    INFO("ReTiF daemon - Daemon started.\n");

    if(retif_daemon_init(&data) < 0)
    {
        ERR("Unexpected error in initialization phase.\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGTTOU, output);
    signal(SIGINT, term);
    retif_daemon_loop(&data);

    return 0;
}
