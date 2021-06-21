/**
 * @file logger.h
 * @author Gabriele Serra
 * @date 19 Nov 2018
 * @brief Redirect log string toward terminal, syslog or nothing
 *
 * This file contains the definition of log macros. Log macros are
 * wrappers for print operation, and they are thought for log printing.
 * Changing the define, the print operation is re-directed towards
 * another output. Changing the level, some print operations are hidden.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <syslog.h>

/**
 * @brief Represent the place where
 * redirecting the output.
 *
 * Define the three different choices,
 * terminal, syslogs or nothing. Please
 * note that stdio.h or syslog.h must be
 * included. Note also that in case of syslog,
 * it must be open before.
 */
enum LOG_HANDLER
{
    LOG_STDOUT, // standard output
    LOG_SYS, // syslog output
    LOG_FILE, // log onto a file
};

/**
 * @brief Represent the log level
 *
 * Change this define what level of loggin should
 * be used during daemon lifetime
 */
enum LOG_LEVEL
{
    ERR = 10,
    WARNING = 20,
    INFO = 30,
    DEBUG = 40,
};

extern struct LOGGER
{
    enum LOG_LEVEL loglvl;
    enum LOG_HANDLER handler;
    FILE *output;
} logger;

#define OUT(level, str, args...)                                               \
    {                                                                          \
        if (logger.handler == LOG_FILE)                                        \
        {                                                                      \
            fprintf(logger.output, str, ##args);                               \
            fflush(logger.output);                                             \
        }                                                                      \
        else if (logger.handler == LOG_SYS)                                    \
        {                                                                      \
            syslog(LOG_DAEMON | LOG_##level, str, ##args);                     \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf(str, ##args);                                               \
        }                                                                      \
    }

#define LOG(level, str, args...)                                               \
    {                                                                          \
        if (level < logger.loglvl)                                             \
            OUT(level, str, ##args);                                           \
    }

#endif /* LOGGER_H */
