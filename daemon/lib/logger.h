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

/**
 * @brief Represent the current choice
 * 
 * Represent the current choice. Change
 * this define to redirect output toward
 * another channel 
 */
#define LOG_STDOUT

/**
 * @brief Represent the log level
 * 
 * Represent the current choice. Change
 * this define what level of loggin should
 * be used during daemon lifetime 
 */
#define LOG_LEVEL 10

/**
 * @brief Define the different choices
 * 
 * Define the three different choices,
 * terminal, syslogs or nothing. Please
 * note that stdio.h or syslog.h must be
 * included. Note also that in case of syslog,
 * it must be open before.
 * 
 * @param str string to be printed
 * @param args variable number of args
 */
#ifdef LOG_STDOUT
    #if LOG_LEVEL >= 40
        #define LOG(str, args...) printf(str, ##args)
    #endif
    #if LOG_LEVEL >= 30
        #define INFO(str, args...) printf(str, ##args)
    #endif
    #if LOG_LEVEL >= 20
        #define WARN(str, args...) printf(str, ##args)
    #endif
    #if LOG_LEVEL >= 10
        #define ERR(str, args...) printf(str, ##args)
    #endif
#elif LOG_SYS
    #if LOG_LEVEL >= 40
        #define LOG(str, args...) syslog(LOG_DAEMON | LOG_DEBUG, str, args)
    #endif
    #if LOG_LEVEL >= 30
        #define INFO(str, args...) syslog(LOG_DAEMON | LOG_INFO, str, args)
    #endif
    #if LOG_LEVEL >= 20
        #define WARN(str, args...) syslog(LOG_DAEMON | LOG_WARNING, str, args)
    #endif
    #if LOG_LEVEL >= 10
        #define ERR(str, args...) syslog(LOG_DAEMON | LOG_ERR, str, args)
    #endif
#else
    #warning Log output was not defined.
#endif

#ifndef LOG
    #define LOG(str, args...)
#endif

#ifndef INFO
    #define INFO(str, args...)
#endif

#ifndef WARN
    #define WARN(str, args...)
#endif

#ifndef ERR
    #define ERR(str, args...)
#endif

#endif /* LOGGER_H */

