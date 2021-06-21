/**
 * @file retif_config.h
 * @author Gabriele Serra
 * @date 03 Jan 2020
 * @brief Allows to configure kernel parameters for real-time scheduling
 */

#include "retif_config.h"
#include "logger.h"
#include "retif_utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @internal
 *
 * Reads rt_period and rt_runtime from proc files and
 * write their values in @p rt_period and @p rt_runtime. Returns
 * -1 in case of errors (leaving untouched params) or 0
 * in case of success.
 *
 * @endinternal
 */
int rtf_config_get_rt_kernel_params(int *rt_period, int *rt_runtime)
{
    FILE *proc_rt_period = fopen(PROC_RT_PERIOD_FILE, "r");
    FILE *proc_rt_runtime = fopen(PROC_RT_RUNTIME_FILE, "r");

    if (proc_rt_period == NULL || proc_rt_runtime == NULL)
    {
        ERR("rtsd error: %s", strerror(errno));
        return -1;
    }

    if (safe_file_read(proc_rt_period, "%d", 1, rt_period) < 0)
        return -1;
    if (safe_file_read(proc_rt_runtime, "%d", 1, rt_runtime) < 0)
        return -1;

    LOG("Reading kernel RT data - PERIOD: %d - RUNTIME: %d\n", *rt_period,
        *rt_runtime);

    fclose(proc_rt_period);
    fclose(proc_rt_runtime);

    return 0;
}

/**
 * @internal
 *
 * Reads rt_period and rt_runtime from proc files and
 * calculate maximum utilization available for RT processes normalized from
 * 0 to 1. Returns -1 in case of errors (leaving untouched param) or 0
 * in case of success.
 *
 * @endinternal
 */
int rtf_config_get_rt_kernel_max_util(float *rt_max_util)
{
    int rt_period, rt_runtime;

    if (rtf_config_get_rt_kernel_params(&rt_period, &rt_runtime) < 0)
        return -1;

    if (rt_runtime == -1)
        *rt_max_util = 1;
    else
        *rt_max_util = rt_runtime / (float) rt_period;

    return 0;
}

/**
 * @internal
 *
 * Writes rt_period and rt_runtime in proc files taking desired
 * values from @p rt_period and @p rt_runtime. Returns
 * -1 in case of errors or 0 in case of success.
 *
 * @endinternal
 */
int rtf_config_set_rt_kernel_params(int rt_period, int rt_runtime)
{
    FILE *proc_rt_period = fopen(PROC_RT_PERIOD_FILE, "w");
    FILE *proc_rt_runtime = fopen(PROC_RT_RUNTIME_FILE, "w");

    if (proc_rt_period == NULL || proc_rt_runtime == NULL)
    {
        WARN("Error opening proc files in writing mode.\n");
        WARN("%s", strerror(errno));
        return -1;
    }

    fprintf(proc_rt_period, "%d", rt_period);
    fprintf(proc_rt_runtime, "%d", rt_runtime);

    LOG("Written kernel RT data - PERIOD: %d - RUNTIME: %d\n", rt_period,
        rt_runtime);

    fclose(proc_rt_period);
    fclose(proc_rt_runtime);

    return 0;
}

/**
 * @internal
 *
 * Restores rt_period and rt_runtime in proc files to default values
 * reading default constants in library. Returns
 * -1 in case of errors or 0 in case of success.
 *
 * @endinternal
 */
int rtf_config_restore_rt_kernel_params()
{
    int ret;

    ret = rtf_config_set_rt_kernel_params(PROC_RT_PERIOD_DEFAULT,
        PROC_RT_RUNTIME_DEFAULT);

    if (ret < 0)
        LOG("Error: Restoring proc file rt parameters failed.\n");
    else
        LOG("Restoring proc file rt parameters successful.\n");

    return ret;
}

/**
 * @internal
 *
 * Reads rr_timeslice from proc files and
 * write its values in @p rr_timeslice. Returns
 * -1 in case of errors (leaving untouched param) or 0
 * in case of success.
 *
 * @endinternal
 */
int rtf_config_get_rr_kernel_param(int *rr_timeslice)
{
    FILE *proc_rr_timeslice = fopen(PROC_RR_TIMESlICE_FILE, "r");

    if (proc_rr_timeslice == NULL)
    {
        LOG("Error opening proc files in reading mode.\n");
        LOG("%s", strerror(errno));
        return -1;
    }

    if (safe_file_read(proc_rr_timeslice, "%d", 1, rr_timeslice) < 0)
        return -1;

    LOG("Reading kernel RR data - TIMESLICE: %d \n", *rr_timeslice);
    fclose(proc_rr_timeslice);

    return 0;
}

/**
 * @internal
 *
 * Writes rr_timeslice in proc files taking desired
 * value from @p rr_timeslice. Returns
 * -1 in case of errors or 0 in case of success.
 *
 * @endinternal
 */
int rtf_config_set_rr_kernel_param(int rr_timeslice)
{
    FILE *proc_rr_timeslice = fopen(PROC_RR_TIMESlICE_FILE, "w");

    if (proc_rr_timeslice == NULL)
    {
        WARN("Error opening proc files in writing mode.\n");
        WARN("%s", strerror(errno));
        return -1;
    }

    fprintf(proc_rr_timeslice, "%d", rr_timeslice);
    LOG("Written kernel RR data - TIMESLICE: %d \n", rr_timeslice);
    fclose(proc_rr_timeslice);

    return 0;
}

/**
 * @internal
 *
 * Restores rr_timeslice in proc files to default values
 * reading default constant in library. Returns
 * -1 in case of errors or 0 in case of success.
 *
 * @endinternal
 */
int rtf_config_restore_rr_kernel_param(int rr_timeslice)
{
    int ret;

    ret = rtf_config_set_rr_kernel_param(PROC_RR_TIMESLICE_DEFAULT);

    if (ret < 0)
        ERR("Error: Restoring proc file rr parameter failed.\n");
    else
        INFO("Restoring proc file rr parameter successful.\n");

    return ret;
}

/**
 * @internal
 *
 * Opens the configuration file, reads the settings specified by the sysadmin
 * and apply new settings, overriding previous one. Returns 0 if successful,
 * -1 in case of errors (unable to open file or to specified settins are not
 * valid)
 *
 * @endinternal
 */
int rtf_config_apply()
{
    FILE *f;
    int rr_timeslice, rt_period, rt_runtime;

    f = fopen(SETTINGS_CFG, "r");

    if (f == NULL)
    {
        ERR("Unable to open cfg file. %s. Are plugins installed?\n",
            strerror(errno));
        return -1;
    }

    go_to_settings_head(f);

    if (extract_num_from_line(f, &rr_timeslice) != 1)
        return -1;
    if (extract_num_from_line(f, &rt_period) != 1)
        return -1;
    if (extract_num_from_line(f, &rt_runtime) != 1)
        return -1;

    if (rtf_config_set_rt_kernel_params(rt_period, rt_runtime) < 0)
        return -1;

    if (rtf_config_set_rr_kernel_param(rr_timeslice) < 0)
        return -1;

    fclose(f);

    return 0;
}
