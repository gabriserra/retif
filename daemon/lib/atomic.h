/**
 * @file atomic.h
 * @author Gabriele Serra
 * @date 21 Nov 2018
 * @brief Defines a pseudo-atomic type on x64 architectures 
 *
 * This file contains the definition of a pseudo-atomic type
 * on x64 architectures. The atomicity is realized with the 
 * volatile keyword. It need the version of GCC greater than 4
 * because it contains some inline methods to work with the atomic
 * type. Note that atomicity on CPU that are not Intel is guaranteed
 * only for the LS 24 bits.
 * Read more: https://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html
 */

#ifndef _ATOMIC_H
#define _ATOMIC_H

#include <stdint.h>

/* Check GCC version, just to be safe */
#if !defined(__GNUC__) || (__GNUC__ < 4) || (__GNUC_MINOR__ < 1) || !defined(__LP64__)
    # error atomic.h works only with GCC newer than version 4.1
#endif

/**
 * @brief Atomic type definition
 * 
 */
typedef struct {
    volatile uint64_t counter;
} atomic_t;

/**
 * @brief Initialize an atomic variable
 * 
 * Initialize an atomic variable with
 * the content passed @i
 * 
 * @param i value that will be assigned to variable
 */
#define ATOMIC_INIT(i)  { (i) }

/**
 * @brief Read atomic variable
 * 
 * Atomically reads the the value of @v
 * 
 * @param v pointer of type atomic_t
 */
#define atomic_read(v) ((v)->counter)

/**
 * @brief Set atomic variable
 * 
 * Copy the value @i into the variable @v
 * 
 * @param v pointer of type atomic_t
 * @param i required value
 */
#define atomic_set(v,i) (((v)->counter) = (i))

/**
 * @brief Add to the atomic variable
 * 
 * Sum the current value of @v with
 * the value of @i in atomic way
 * 
 * @param i integer value to add
 * @param v pointer of type atomic_t
 */
static inline void atomic_add(uint64_t i, atomic_t *v) {
	(void)__sync_add_and_fetch(&v->counter, i);
}

/**
 * @brief Subtract the atomic variable
 * 
 * Subtract the current value of @v
 * with the value of @i in atomic way
 * 
 * @param i integer value to subtract
 * @param v pointer of type atomic_t
 */
static inline void atomic_sub(uint64_t i, atomic_t *v) {
	(void)__sync_sub_and_fetch(&v->counter, i);
}

/**
 * @brief Subtract value from variable and test result
 * 
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 * 
 * @param i integer value to subtract
 * @param v pointer of type atomic_t
 * @return 1 if the result is 0, 0 otherwise
 */
static inline int atomic_sub_and_test(uint64_t i, atomic_t *v) {
	return !(__sync_sub_and_fetch(&v->counter, i));
}

/**
 * @brief Increment atomic variable
 * 
 * Atomically increments @v by 1.
 * 
 * @param v pointer of type atomic_t
 */
static inline void atomic_inc(atomic_t *v) {
	(void)__sync_fetch_and_add(&v->counter, 1);
}

/**
 * @brief decrement atomic variable
 *
 * Atomically decrements @v by 1.
 * 
 * @param v: pointer of type atomic_t
 */
static inline void atomic_dec(atomic_t *v) {
	(void)__sync_fetch_and_sub(&v->counter, 1);
}

/**
 * @brief Decrement and test
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, 
 * or false for all other cases.
 * 
 * @param v pointer of type atomic_t
 * @return 1 if the result is 0, 0 otherwise
 * 
 */
static inline int atomic_dec_and_test(atomic_t *v) {
	return !(__sync_sub_and_fetch(&v->counter, 1));
}

/**
 * @brief Increment and test
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, 
 * or false for all other cases.
 * 
 * @param v pointer of type atomic_t
 * @return 1 if the result is 0, 0 otherwise
 */
static inline int atomic_inc_and_test(atomic_t *v) {
	return !(__sync_add_and_fetch(&v->counter, 1));
}

#endif
