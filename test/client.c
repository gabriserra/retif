#include <errno.h>
#include <retif.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define T_BUDGET_MIN 10
#define T_BUDGET_MAX 40
#define T_PERIOD_MIN 1024
#define T_PERIOD_MAX 2048

#define RAND_SEED 1

void print_err(char *message)
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
    if (rtf_connect() < 0)
        print_err("# Error - Unable to connect with daemon.\n");
    else
        printf("# Connected with daemon.\n");
}

int main(int argc, char *argv[])
{
    uint32_t ret;
    struct rtf_params p = RTF_PARAM_INIT;
    struct rtf_task t = RTF_TASK_INIT;

    daemon_connect();

    rtf_params_init(&p);
    rtf_task_init(&t);

    rtf_params_set_period(&p, rand_bounded(T_PERIOD_MIN, T_PERIOD_MAX));
    rtf_params_set_runtime(&p, rand_bounded(T_BUDGET_MIN, T_BUDGET_MAX));
    rtf_params_set_deadline(&p,
        rand_bounded(rtf_params_get_runtime(&p), rtf_params_get_period(&p)));

    ret = rtf_task_create(&t, &p);

    if (ret < 0)
        print_err("# Daemon was unable to serve the request.");

    rtf_task_attach(&t, getpid());

    while (!getc(stdin))
        ;

    rtf_task_release(&t);

    return 0;
}
