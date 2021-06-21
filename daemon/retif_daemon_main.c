/**
 * @file daemon.c
 * @author Gabriele Serra
 * @date 05 Jan 2020
 * @brief Contains daemon entry & exit points
 */

#include "logger.h"
#include "retif_cli.h"
#include "retif_daemon.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * ReTif version and mainteiner info
 */
const char *retif_version =
    "ReTif 0.1.0";
const char *retif_bug_address =
    "<gabriele.serra@santannapisa.it>, <gabriele.ara@santannapisa.it>";

struct rtf_daemon data;

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;

    switch (key)
    {
    case 'l':
        args->level = atoi(arg);
        break;
    case 'o':
        args->output_file = arg;
        break;
    case ARGP_KEY_ARG:
        /* Too many arguments. */
        if (state->arg_num >= 1)
            argp_usage(state);
        break;
    case ARGP_KEY_END:
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/**
 * @brief When daemon is signaled with a SIGINT, tear down it
 */
void term()
{
    LOG(INFO, "\nReTiF daemon was interrupted. It will destroy data and stop.\n");
    rtf_daemon_destroy(&data);
    exit(EXIT_SUCCESS);
}

/**
 * @brief When daemon is signaled with a SIGTTOU, dump debug info
 */
void output()
{
    LOG(INFO, "\n--------------------------\n");
    LOG(INFO, "ReTiF daemon info dump\n");
    LOG(INFO, "--------------------------\n");
    rtf_daemon_dump(&data);
}

/**
 * @brief Main daemon routine, initializes data and starts loop
 */
int main(int argc, char *argv[])
{
    struct arguments arguments;

    arguments.level = 20;
    arguments.output_file = "\0";

    struct argp argp = {options, parse_opt, 0, doc};

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    logger.loglvl = arguments.level;

    if (*arguments.output_file != '\0')
    {
        logger.handler = arguments.output_file;
        logger.output = fopen(arguments.output_file, 'a');
    }

    LOG(INFO, "ReTiF daemon - Daemon started.\n");

    if (rtf_daemon_init(&data) < 0)
    {
        LOG(ERR, "Unexpected error in initialization phase.\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGTTOU, output);
    signal(SIGINT, term);
    rtf_daemon_loop(&data);

    return 0;
}
