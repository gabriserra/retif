/**
 * This benchmark will spawn 1024 tasks and each of them will perform a
 * request to the daemon. The connection is performed only by the main process
 * hence the cost is not calculated at all. The output will be print into 
 * the file passed as the first argument.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <librts.h>
#include <errno.h>
#include <string.h>

#include "rte_cycles.h"

#define TSC
#define B_NUM           500
#define T_NUM           1024

#define T_BUDGET_MIN    1
#define T_BUDGET_MAX    4
#define T_PERIOD_MIN    1024
#define T_PERIOD_MAX    2048

#define RAND_SEED       1

void print_err(char* message)
{
    printf("%s", message);
    printf("%s", strerror(errno));
    exit(-1);
}

uint32_t rand_bounded(uint32_t min, uint32_t max)
{
    return min + (rand() % max);
}

void daemon_connect()
{
    if (rts_daemon_connect() < 0)
        print_err("# Error - Unable to connect with daemon.\n");
    else
        printf("# Connected with daemon.\n");
}

int main(int argc, char* argv[]) 
{
    FILE* output;
    uint32_t ret;
    struct rts_params p;
    struct rts_task t[T_NUM];

    if (argc < 2)
        print_err("# Error - Wrong number of args.\n");

    output = fopen(argv[1], "a");

    if (output == NULL)
        print_err("# Error - Unable to open output file.\n");

    #ifdef TSC
    uint64_t tsc_before, tsc_after, tsc_freq;
    tsc_freq = rte_get_tsc_hz();
    printf("# TSC frequency: %ld\n", tsc_freq);
    #endif

    daemon_connect();

    printf("# Benchmark started ..... !\n");
    srand(RAND_SEED);

    for (int bn = 0; bn < B_NUM; bn++)
    {
        printf("# Benchmark - %d \n", bn);

        for (int i = 0; i < T_NUM; i++)
        {
            rts_params_init(&p);
            rts_task_init(&(t[i]));

            rts_params_set_period(&p, rand_bounded(T_PERIOD_MIN, T_PERIOD_MAX));
            rts_params_set_runtime(&p, rand_bounded(T_BUDGET_MIN, T_BUDGET_MAX));

            #ifndef TSC
                struct timespec tp_before;
                struct timespec tp_after;
                clock_gettime(CLOCK_MONOTONIC_RAW, &tp_before);
                rts_task_create(&(t[i]), &p);
                clock_gettime(CLOCK_MONOTONIC_RAW, &tp_after);
                fprintf(output, "%ld,", tp_after.tv_nsec - tp_before.tv_nsec);
            #else
                tsc_before = rte_get_tsc_cycles();
                ret = rts_task_create(&(t[i]), &p);
                tsc_after = rte_get_tsc_cycles();
                fprintf(output, "%ld,", rte_get_tsc_elapsed(tsc_before, tsc_after, tsc_freq));
            #endif

            if (ret < 0)
                print_err("# Benchmar error - Unable to create task");

        }

        for (int i = 0; i < T_NUM; i++)
        {
            rts_task_destroy(&(t[i]));
        }

        fprintf(output, "\n");
    }

    fclose(output);
    printf("# Benchmark ended ..... !\n");

    return 0;
}
