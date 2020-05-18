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
#include "rts_daemon.h"

struct rts_daemon data;

/**
 * @brief When daemon is signaled with a SIGINT, tear down it
 */
void term() 
{
    INFO("\nRTS daemon was interrupted. It will destroy data and stop.\n");
    rts_daemon_destroy(&data);    
    exit(EXIT_SUCCESS);
}

/**
 * @brief Main daemon routine, initializes data and starts loop
 */
int main(int argc, char* argv[]) 
{
    INFO("rts-daemon - Daemon started.\n");
    
    if(rts_daemon_init(&data) < 0)
    {
        ERR("Unexpected error in initialization phase.\n");
        exit(EXIT_FAILURE);
    }
    
    signal(SIGINT, term);
    rts_daemon_loop(&data);
    
    return 0;
}
