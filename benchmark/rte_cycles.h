#include <stdint.h>

/**
 * Return TSC tick frequency expressed in Hz
 */
uint64_t rte_get_tsc_hz();

/**
 * Return the current value of Time Stamp Counter
 * Note: x86 implementation
 */
uint64_t rte_get_tsc_cycles(void);

/**
 * Calculate the elapsed microseconds between two TSC cycles at a given frequency
 */
uint64_t rte_get_tsc_elapsed(uint64_t tsc_before, uint64_t tsc_after, uint64_t tsc_freq);