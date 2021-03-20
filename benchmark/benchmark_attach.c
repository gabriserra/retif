/**
 * This benchmark will spawn 4 tasks each of them specifies the preferred plugin
 * and performs a request to the daemon. In this benchmark we want to evaluate
 * the cost of attaching a thread and schedule it. The output will be print into 
 * the file passed as the first argument.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <librts.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "rte_cycles.h"

#define T_NUM           4
#define B_NUM           500

#define EDF "EDF"
#define RM  "RM"
#define FP  "FP"
#define RR  "RR"

#define T_BUDGET_MIN    1
#define T_BUDGET_MAX    4
#define T_PERIOD_MIN    1024
#define T_PERIOD_MAX    2048
#define T_PRIORI_MIN    10
#define T_PRIORI_MAX    50

#define RAND_SEED       1

volatile uint64_t spawned = 0;
uint64_t tids[T_NUM];

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

char* get_scheduler(int i)
{
    if (i == 0)
        return EDF;
    else if (i == 1)
        return RM;
    else if (i == 2)
        return FP;
    else
        return RR;
}

void daemon_connect()
{
    if (rts_daemon_connect() < 0)
        print_err("# Error - Unable to connect with daemon.\n");
    else
        printf("# Connected with daemon.\n");
}

void* do_something(void* t_id)
{
    tids[(long)t_id] = syscall(SYS_gettid);
    spawned++;
    while(1);

    return NULL;
}

int main(int argc, char* argv[]) 
{
    FILE* output;
    uint32_t ret;
    struct rts_params p;
    struct rts_task t[T_NUM];
    pthread_t pt[T_NUM];

    if (argc < 2)
    {
        printf("# Error - Wrong number of args. Usage %s <filename>\n", argv[0]);
        return -1;
    }

    output = fopen(argv[1], "a");

    if (output == NULL)
        print_err("# Error - Unable to open output file.\n");

    uint64_t tsc_before, tsc_after, tsc_freq;
    tsc_freq = rte_get_tsc_hz();
    printf("# TSC frequency: %ld\n", tsc_freq);
    
    daemon_connect();

    for (int i = 0; i < T_NUM; i++)
        if (pthread_create(&pt[i], NULL, do_something, NULL) != 0)
            print_err("# Error - Unable to create pthreads.\n");

    // wait till other threads are ready
    while(spawned != T_NUM);
    
    printf("# Benchmark started ..... !\n");
    srand(RAND_SEED);

    for (int bn = 0; bn < B_NUM; bn++)
    {
        printf("# Benchmark - %d \n", bn);

        for (int i = 0; i < T_NUM; i++)
        {
            rts_params_init(&p);
            rts_task_init(&(t[i]));

            rts_params_set_scheduler(&p, get_scheduler(i));
            rts_params_set_period(&p, rand_bounded(T_PERIOD_MIN, T_PERIOD_MAX));
            rts_params_set_runtime(&p, rand_bounded(T_BUDGET_MIN, T_BUDGET_MAX));
            rts_params_set_priority(&p, rand_bounded(T_PRIORI_MIN, T_PRIORI_MAX));

            ret = rts_task_create(&(t[i]), &p);

            if (ret < 0)
                print_err("# Benchmark error - Unable to create task");

            tsc_before = rte_get_tsc_cycles();
            ret = rts_task_attach(&(t[i]), tids[i]);
            tsc_after = rte_get_tsc_cycles();

            if (i == T_NUM -1)
                fprintf(output, "%ld", rte_get_tsc_elapsed(tsc_before, tsc_after, tsc_freq));
            else
                fprintf(output, "%ld,", rte_get_tsc_elapsed(tsc_before, tsc_after, tsc_freq));

            if (ret < 0)
                print_err("# Benchmark error - Unable to attach thread");
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
