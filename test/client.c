#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <retif.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define T_BUDGET_MIN    10
#define T_BUDGET_MAX    40
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
    if (retif_daemon_connect() < 0)
        print_err("# Error - Unable to connect with daemon.\n");
    else
        printf("# Connected with daemon.\n");
}

int main(int argc, char* argv[])
{
    uint32_t ret;
    struct retif_params p;
    struct retif_task t;

    daemon_connect();

    retif_params_init(&p);
    retif_task_init(&t);

    retif_params_set_period(&p, rand_bounded(T_PERIOD_MIN, T_PERIOD_MAX));
    retif_params_set_runtime(&p, rand_bounded(T_BUDGET_MIN, T_BUDGET_MAX));

    ret = retif_task_create(&t, &p);

    if (ret < 0)
        print_err("# Daemon was unable to serve the request.");

    while(!getc(stdin));

    retif_task_destroy(&t);

    return 0;
}
