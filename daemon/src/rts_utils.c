#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <sys/sysinfo.h>
#include "logger.h"
#include "rts_utils.h"

// -----------------------------------------------------------------------------
// FILE, CONFIG & PLUGIN UTILS (FUNCTIONS)
// -----------------------------------------------------------------------------

/**
 * @internal
 * 
 * Gets a line from the configuration file and returns the type of line
 * encountered based on the first char (COMMENT, SETTINGS_HEAD, SETTINGS_BODY,
 * NEWLINE)
 * 
 * @endinternal
 */
int get_cfg_line(FILE* f, char buffer[CFG_COLUMN_MAX])
{
    enum CFG_LINE ln;

    if (fgets(buffer, CFG_COLUMN_MAX, f) == NULL)
    {
        WARN("Read null from CFG file. Bad configuration.\n");
        return -1;
    }

    switch (buffer[0]) 
    {
        case CFG_COMMENT_TOKEN:
            ln = COMMENT;
            break;
        case CFG_SETTINGS_TOKEN:
            ln = SETTINGS_HEAD;
            break;
        case '\n':
            ln = NEWLINE;
            break;
        default:
            ln = SETTINGS_BODY;
    }

    return ln;
}

/**
 * @internal
 * 
 * Takes a configuration file in input and skip all rows that do
 * not represent useful information (comment and/or newline ..)
 * 
 * @endinternal
 */
void go_to_settings_head(FILE* f) 
{
    char buffer[CFG_COLUMN_MAX];
    enum CFG_LINE ln;
    
    while(1) 
    {
        ln = get_cfg_line(f, buffer);

        if(ln == -1)
            break;
        
        if(ln == SETTINGS_HEAD)
            break;
    }
}

/**
 * @internal
 * 
 * Take as input the configuration file with the stream positioned at
 * SETTINGS_HEAD and get the number of lines. Returns the number of rows read.
 * 
 * @endinternal
 */
int count_num_of_settings(FILE* f) 
{
    long start_pos;
    int num_of_elem;
    char buffer[CFG_COLUMN_MAX];
    
    start_pos = ftell(f);
    num_of_elem = 0;
    
    while(!feof(f)) 
    {
        if(fgets(buffer, CFG_COLUMN_MAX, f) != NULL)
            num_of_elem++;
    }
        
    fseek(f, start_pos, SEEK_SET);

    return num_of_elem;
}

int safe_file_read(FILE* f, char* format, int argnum, ...) 
{
    int ret;
    va_list arglist;
    
    va_start(arglist, argnum);
    ret = vfscanf(f, format, arglist);
    va_end(arglist);

    if (ret != argnum)
    {
        WARN("Unable to read from file.\n");
        return -1;
    }

    return 0;
}

int extract_num_from_line(FILE* f, int* content)
{
    char buffer[CFG_COLUMN_MAX];

    if (fgets(buffer, CFG_COLUMN_MAX, f) == NULL)
    {
        WARN("Unable to read from file.\n");
        return -1;
    }

    return sscanf(buffer, "%d", content) == 1;
}

// -----------------------------------------------------------------------------
// MEMORY UTILS
// -----------------------------------------------------------------------------

/**
 * @internal
 * 
 * Allocates @p nmbemb slots of @p size bytes of memory and initializes the 
 * previous allocated memory slots copying @p size bytes from @p src. Returns 
 * NULL if was not possible to allocate memory or the pointer to the memory area
 * allocated in case of success.
 * 
 * @endinternal
 */
void* array_alloc_wcopy(uint32_t nmemb, size_t size, const void* src)
{
    void* dest = calloc(nmemb, size);

    if (dest == NULL)
        return NULL;

    for (int i = 0; i < nmemb; i++)
        memcpy(dest+(i*size), src, size);

    return dest;
}

// -----------------------------------------------------------------------------
// TIME UTILS (FUNCTIONS)
// -----------------------------------------------------------------------------

void time_add_us(struct timespec *t, uint64_t us) {
    t->tv_sec += MICRO_TO_SEC(us);               
    t->tv_nsec += MICRO_TO_NANO(us % EXP6);     
	
    if (t->tv_nsec > EXP9) { 
        t->tv_nsec -= EXP9; 
        t->tv_sec += 1;
    }
}

void time_add_ms(struct timespec *t, uint32_t ms) {
    t->tv_sec += MILLI_TO_SEC(ms);
    t->tv_nsec += MILLI_TO_NANO(ms % EXP3);
	
    if (t->tv_nsec > EXP9) { 
        t->tv_nsec -= EXP9; 
        t->tv_sec += 1;
    }
}

int time_cmp(struct timespec* t1, struct timespec* t2) {
    if (t1->tv_sec > t2->tv_sec) 
            return 1; 
    if (t1->tv_sec < t2->tv_sec) 
            return -1;

    if (t1->tv_nsec > t2->tv_nsec) 
            return 1; 
    if (t1->tv_nsec < t2->tv_nsec) 
            return -1; 

    return 0;
}

void time_copy(struct timespec* td, struct timespec* ts) {
    td->tv_sec  = ts->tv_sec;
    td->tv_nsec = ts->tv_nsec;
}

uint64_t timespec_to_us(struct timespec *t) {
    uint64_t us;
    
    us = SEC_TO_MICRO(t->tv_sec);
    us += NANO_TO_MICRO(t->tv_nsec);
	
    return us;
}

void us_to_timespec(struct timespec *t, uint64_t us) {
    t->tv_sec = MICRO_TO_SEC(us);               
    t->tv_nsec = MICRO_TO_NANO(us % EXP6);
}

uint32_t timespec_to_ms(struct timespec *t) {
    uint32_t ms;
    
    ms = SEC_TO_MILLI(t->tv_sec);
    ms += NANO_TO_MILLI(t->tv_nsec);
	
    return ms;
}

void ms_to_timespec(struct timespec *t, uint32_t ms) {
    t->tv_sec = MILLI_TO_SEC(ms);               
    t->tv_nsec = MILLI_TO_NANO(ms % EXP3);
}

struct timespec get_time_now(clockid_t clk) {
    struct timespec ts;
    
    clock_gettime(clk, &ts);
    return ts;
}

void get_time_now2(clockid_t clk, struct timespec* t) {
    clock_gettime(clk, t);
}

uint32_t get_time_now_ms(clockid_t clk) {
    struct timespec ts;
    
    clock_gettime(clk, &ts);
    return timespec_to_ms(&ts);
}

struct timespec get_thread_time() {
    struct timespec ts;
    
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return ts;
}

uint32_t get_thread_time_ms() {
    struct timespec ts;
    
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return timespec_to_ms(&ts);
}

void compute_for(uint32_t exec_milli_max) {
    uint32_t exec_milli;
    struct timespec t_curr;
    struct timespec t_end;
    
    exec_milli = rand() % exec_milli_max; 

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_end);
    time_add_ms(&t_end, exec_milli);
    
    while(1) {
        __asm__ ("nop"); // simulate computation of something..
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t_curr);
        
        if(time_cmp(&t_curr, &t_end) > 0)
            break;
    } 
}

void wait_next_activation(struct timespec* t_act, uint32_t period_milli) {
    time_add_ms(t_act, period_milli);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t_act, NULL);
}

void set_timer(uint32_t milli) {
    struct itimerval t;
    t.it_interval.tv_sec = 0;
    t.it_interval.tv_usec = MILLI_TO_MICRO(milli);

    setitimer(ITIMER_REAL, &t, NULL);
}

int get_nprocs2(void) 
{
#ifdef N_PROC_OVERRIDE
    return N_PROC_OVERRIDE;
#else
    return get_nprocs();
#endif
}