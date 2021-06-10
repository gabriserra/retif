
#ifndef RETIF_UTILS_H
#define RETIF_UTILS_H

#include <time.h>
#include <stdint.h>
#include <stdio.h>

#define _GNU_SOURCE
#define N_PROC_OVERRIDE 20

// -----------------------------------------------------------------------------
// DEFINE TIME CONSTANT (IF UNAVAILABLE IN LIBC)
// -----------------------------------------------------------------------------

#ifndef __clockid_t_defined
#define __clockid_t_defined 1

typedef __clockid_t clockid_t;

#endif

#ifndef CLOCK_MONOTONIC
    #define CLOCK_MONOTONIC		        1
#endif
#ifndef CLOCK_THREAD_CPUTIME_ID
    #define CLOCK_THREAD_CPUTIME_ID		3
#endif
#ifndef TIMER_ABSTIME
    #define TIMER_ABSTIME			    0x01
#endif


// -----------------------------------------------------------------------------
// FILE, CONFIG & PLUGIN UTILS (MACRO - TYPES)
// -----------------------------------------------------------------------------

#define CFG_COMMENT_TOKEN           '#'
#define CFG_SETTINGS_TOKEN          '!'
#define CFG_COLUMN_MAX              82

enum CFG_LINE
{
    NEWLINE,
    COMMENT,
    SETTINGS_HEAD,
    SETTINGS_BODY
};

// -----------------------------------------------------------------------------
// TIME UTILS (MACRO - TYPES)
// -----------------------------------------------------------------------------

#define EXP3 1000
#define EXP6 1000000
#define EXP9 1000000000

#define SEC_TO_MILLI(sec) sec * EXP3
#define SEC_TO_MICRO(sec) sec * EXP6
#define SEC_TO_NANO(sec) sec * EXP9

#define MILLI_TO_SEC(milli) milli / EXP3
#define MILLI_TO_MICRO(milli) milli * EXP3
#define MILLI_TO_NANO(milli) milli * EXP6

#define MICRO_TO_SEC(micro) micro / EXP6
#define MICRO_TO_MILLI(micro) micro / EXP3
#define MICRO_TO_NANO(micro) micro * EXP3

#define NANO_TO_SEC(nano) nano / EXP9
#define NANO_TO_MILLI(nano) nano / EXP6
#define NANO_TO_MICRO(nano) nano / EXP3

// -----------------------------------------------------------------------------
// FILE, CONFIG & PLUGIN UTILS (FUNCTIONS)
// -----------------------------------------------------------------------------

/**
 * @brief Reads a line from configuration file and discriminate line-type
 *
 * Gets a line from the configuration file and returns the type of line
 * encountered based on the first char (COMMENT, SETTINGS_HEAD, SETTINGS_BODY,
 * NEWLINE)
 *
 * @param f pointer to FILE* opened configuration file
 * @param buffer the buffer that will contain the line read from the file
 * @return type of line (COMMENT = 0, SETTINGS_HEAD, SETTINGS_BODY, NEWLINE)
 */
int get_cfg_line(FILE* f, char buffer[CFG_COLUMN_MAX]);

/**
 * @brief Seek the stream, skipping all NEWLINE and COMMENT rows
 *
 * Take a configuration file in input and skip all rows that do
 * not represent useful information (comment and/or newline ..)
 *
 * @param f pointer to FILE* opened configuration file
 */
void go_to_settings_head(FILE* f);

/**
 * @brief Reads the number of option and compares with the number given
 *
 * Take as input the configuration file with the stream positioned at
 * SETTINGS_HEAD and get the number of lines. Returns the number of rows read.
 *
 * @param f pointer to FILE* opened configuration file
 * @param settingsnum the expected number of settings found
 * @return number of rows read
 */
int count_num_of_settings(FILE* f);

int safe_file_read(FILE* f, char* format, int argnum, ...);

int extract_num_from_line(FILE* f, int* content);

// -----------------------------------------------------------------------------
// MEMORY UTILS
// -----------------------------------------------------------------------------

/**
 * @brief Allocates memory for an array and initializes its member
 *
 * Allocates @p nmbemb slots of @p size bytes of memory and initializes the
 * previous allocated memory slots copying @p size bytes from @p src. Returns
 * NULL if was not possible to allocate memory or the pointer to the memory area
 * allocated in case of success.
 *
 * @param nmemb number of slots for the array
 * @param size number of bytes of memory to allocate / initialize
 * @param src pointer to memory area that will be used as data source
 * @return NULL if memory is unavailable, pointer to mem area otherwise
 */
void* array_alloc_wcopy(uint32_t nmemb, size_t size, const void* src);

// -----------------------------------------------------------------------------
// TIME UTILS (FUNCTIONS)
// -----------------------------------------------------------------------------

void time_add_us(struct timespec *t, uint64_t us);

void time_add_ms(struct timespec *t, uint32_t ms);

int time_cmp(struct timespec* t1, struct timespec* t2);

void time_copy(struct timespec* td, struct timespec* ts);

uint64_t timespec_to_us(struct timespec *t);

void us_to_timespec(struct timespec *t, uint64_t us);

uint32_t timespec_to_ms(struct timespec *t);

void ms_to_timespec(struct timespec *t, uint32_t ms);

struct timespec get_time_now(clockid_t clk);

void get_time_now2(clockid_t clk, struct timespec* t);

uint32_t get_time_now_ms(clockid_t clk);

struct timespec get_thread_time();

uint32_t get_thread_time_ms();

void compute_for(uint32_t exec_milli_max);

void wait_next_activation(struct timespec* t_act, uint32_t period_milli);

void set_timer(uint32_t milli);

int get_nprocs2(void);


#endif	// RETIF_UTILS_H
