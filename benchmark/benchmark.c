/**
 * This benchmark will spawn 1024 tasks and each of them will perform a
 * request to the daemon. The connection is performed only by the main process
 * hence the cost is not calculated at all. The output will be print into 
 * the file passed as the first argument.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <librts.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <sys/sysinfo.h>

#include "rte_cycles.h"

#define TSC
#define B_NUM           500
#define T_NUM           1024

#define T_BUDGET_MIN    1
#define T_BUDGET_MAX    4
#define T_PERIOD_MIN    1024
#define T_PERIOD_MAX    2048
#define T_PRIORI_MIN    10
#define T_PRIORI_MAX    50

#define RAND_SEED       1

volatile uint64_t spawned = 0;
uint64_t tids[T_NUM];

struct sched_attr 
{
	__u32 size;
	__u32 sched_policy;
	__u64 sched_flags;
	__s32 sched_nice;
	__u32 sched_priority;
	__u64 sched_runtime;
	__u64 sched_deadline;
    __u64 sched_period;
};

int schedule(struct rts_params* p, int tid, int policy)
{
    struct sched_attr attr;
    cpu_set_t my_set;

    memset(&attr, 0, sizeof(attr));

    CPU_ZERO(&my_set);
    CPU_SET(2, &my_set);

    if(sched_setaffinity(tid, sizeof(cpu_set_t), &my_set) < 0)
        return -1;

    attr.size = sizeof(attr);
    attr.sched_policy   = policy;
    attr.sched_priority = p->priority;
    attr.sched_runtime  = MICRO_TO_NANO(p->runtime);
    attr.sched_deadline = MICRO_TO_NANO(p->period);
    attr.sched_period   = MICRO_TO_NANO(p->period);
    return sched_setscheduler(tid, policy, (struct sched_param*)&attr);
}

int deschedule(int tid)
{
    struct sched_param attr;
    cpu_set_t my_set;

    memset(&attr, 0, sizeof(attr));

    CPU_ZERO(&my_set);
    
    for(int i = 0; i < get_nprocs(); i++)
        CPU_SET(i, &my_set);
    
    if(sched_setaffinity(tid, sizeof(cpu_set_t), &my_set) < 0)
        return -1;
    
    attr.sched_priority = 0;
    
    if(sched_setscheduler(tid, SCHED_OTHER, &attr) < 0)
        return -1;
    
    return 0;
}

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

void* do_something(void* t_id)
{
    tids[(long)t_id] = syscall(SYS_gettid);
    __sync_synchronize();
    spawned++;
    sleep(1000000);

    return NULL;
}

int main(int argc, char* argv[]) 
{
    FILE* output;
    uint32_t ret;
    struct rts_params p;
    struct rts_task t[T_NUM];
    pthread_t pt[T_NUM];

    if (argc < 4)
    {
        printf("# Error - Wrong number of args. Usage %s <filename> <cpu> <test-type> <policy>\n", argv[0]);
        return -1;
    }

    if (access(argv[1], F_OK) == 0)
    {
        output = fopen(argv[1], "a");
    }
    else
    {
        output = fopen(argv[1], "w");
        fprintf(output, "cpu,rip");
        if (strcmp(argv[3], "attach") == 0)
            for (int i = 0; i < T_NUM; i++) fprintf(output, ",t%d", i);
        else
            for (int i = 0; i < T_NUM; i++) fprintf(output, ",t%d_sys,t%d", i, i);
        fprintf(output, "\n");
    }

    #ifdef TSC
    uint64_t tsc_before, tsc_after, tsc_freq;
    tsc_freq = rte_get_tsc_hz();
    printf("# TSC frequency: %ld\n", tsc_freq);
    #endif

    daemon_connect();

    if (strcmp(argv[3], "attach") == 0)
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
        fprintf(output, "%s,%d", argv[2], bn+1);

        for (int i = 0; i < T_NUM; i++)
        {
            rts_params_init(&p);
            rts_task_init(&(t[i]));

            rts_params_set_period(&p, rand_bounded(T_PERIOD_MIN, T_PERIOD_MAX));
            rts_params_set_runtime(&p, rand_bounded(T_BUDGET_MIN, T_BUDGET_MAX));
            rts_params_set_priority(&p, rand_bounded(T_PRIORI_MIN, T_PRIORI_MAX));

            #ifndef TSC
                struct timespec tp_before;
                struct timespec tp_after;
                clock_gettime(CLOCK_MONOTONIC_RAW, &tp_before);
                ret = rts_task_create(&(t[i]), &p);
                clock_gettime(CLOCK_MONOTONIC_RAW, &tp_after);
                if (strcmp(argv[3], "create") == 0)
                    fprintf(output, ",%ld,", tp_after.tv_nsec - tp_before.tv_nsec);
            #else
                tsc_before = rte_get_tsc_cycles();
                ret = rts_task_create(&(t[i]), &p);
                tsc_after = rte_get_tsc_cycles();
                if (strcmp(argv[3], "create") == 0)
                    fprintf(output, ",%ld", rte_get_tsc_elapsed(tsc_before, tsc_after, tsc_freq));
            #endif

            if (ret < 0)
                print_err("# Benchmark error - Unable to create task");

            if (strcmp(argv[3], "attach") == 0)
            {
                #ifndef TSC
                    clock_gettime(CLOCK_MONOTONIC_RAW, &tp_before);
                    ret = schedule(&p, tids[i], atoi(argv[4]));
                    clock_gettime(CLOCK_MONOTONIC_RAW, &tp_after);
                    fprintf(output, ",%ld,", tp_after.tv_nsec - tp_before.tv_nsec);
                #else
                    tsc_before = rte_get_tsc_cycles();
                    ret = schedule(&p, tids[i], atoi(argv[4]));
                    tsc_after = rte_get_tsc_cycles();
                    fprintf(output, ",%ld", rte_get_tsc_elapsed(tsc_before, tsc_after, tsc_freq));
                #endif

                if (ret < 0)
                    print_err("# Benchmark error - Unable to schedule");
                
                ret = deschedule(tids[i]);

                if (ret < 0)
                    print_err("# Benchmark error - Unable to deschedule");

                #ifndef TSC
                    clock_gettime(CLOCK_MONOTONIC_RAW, &tp_before);
                    ret = rts_task_attach(&(t[i]), tids[i]);
                    clock_gettime(CLOCK_MONOTONIC_RAW, &tp_after);
                    fprintf(output, ",%ld,", tp_after.tv_nsec - tp_before.tv_nsec);
                #else
                    tsc_before = rte_get_tsc_cycles();
                    ret = rts_task_attach(&(t[i]), tids[i]);
                    tsc_after = rte_get_tsc_cycles();
                    fprintf(output, ",%ld", rte_get_tsc_elapsed(tsc_before, tsc_after, tsc_freq));
                #endif

                if (ret < 0)
                    print_err("# Benchmark error - Unable to attach thread");
            }
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
