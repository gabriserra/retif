/**
 * @file shatomic.h
 * @author Gabriele Serra
 * @date 10 Nov 2018
 * @brief Realizes a pseudo-atomic shared memory segment 
 *
 * This file contains the interface of shatomic component.
 * It realizes a pseudo-atomic shared memory segment that
 * can be written/read by multiple process without have to 
 * worry about mutual exclusion. Atomicity is not guaranteed
 * for values over 24, so please be careful.
 */

#ifndef SHATOMIC_H
#define SHATOMIC_H

#include "atomic.h"
#include <sys/ipc.h>

/**
 * @defgroup KEY_DEFINE
 * @brief Describe the key length and the template used to generate it
 */
#define KEY_LEN 12
#define KEY_TEMPLATE ".key-XXXXXX"

/**
 * @brief The mask privileges assigned to shared memory segment
 */
#define PRIVILEGES (0666)

/**
 * @brief Represent the shatomic object
 * 
 * The structure shatomic contains the shared memory
 * segment id, the key used to access to the segment,
 * the number of value allocated and the pointer to
 * an array of that value.
 */
struct shatomic {
    int         id;     /** Shared memory identification number */
    key_t       key;    /** Opaque type (often int32), used to access to seg.*/
    int         nvalue; /** Number of value allocated */
    atomic_t*   value;  /** Pointer to first atomic_t value (array) */
};

/**
 * @brief Initialize the shatomic structure
 * 
 * Initialize the shatomic structure, filling
 * with 0s
 * 
 * @param mem pointer to shatomic struct to be initialized
 */
void shatomic_init(struct shatomic* mem);

/**
 * @brief Create a shared atomic segment of memory
 * 
 * Generate the memory segment key and allocate
 * a shared atomic memory segment of @nvalue number
 * of value.
 * 
 * @param mem pointer to shatomic struct to be created
 * @param nvalue number of value that must be allocated
 * @return -1 in case of error, 0 if the operation was successful
 */
int shatomic_create(struct shatomic* mem, int nvalue);

/**
 * @brief Read and return the key of an existent shared memory segment
 * 
 * Read and return the key of an existent shared memory segment.
 * Memory segment must be already created, otherwise the function
 * returns 0
 * 
 * @param mem pointer to shatomic struct
 * @return the key associated with the memory segment, or 0.
 */
key_t shatomic_getkey(struct shatomic* mem);

/**
 * @brief Get an existent memory segment
 * 
 * Get an existent memory segment that have key @key
 * and @num as number of element.
 * 
 * @param mem pointer to a new shatomic struct
 * @param key the key of the shared memory segment that will obtained
 * @param num the number of element of the memory segment
 * @return -1 in case of error, 0 otherwise.
 */
int shatomic_use(struct shatomic* mem, key_t key, int num);

/**
 * @brief Attaches to a previous created shared memory segment
 * 
 * Attaches to a previous created shared memory segment. Memory
 * segment must be previous created with shatomic_create(...) or
 * get with shatomic_use(...)
 * 
 * @param mem pointer to shatomic struct
 */
int shatomic_attach(struct shatomic* mem);

/**
 * @brief Destroy a shared memory segment
 * 
 * Removes a shared memory segment and deallocate
 * memory to free space.
 * 
 * @param mem pointer to shatomic struct that must be destroyed
 */
int shatomic_destroy(struct shatomic* mem);

/**
 * @brief Detach from a shared memory segment
 * 
 * Execute a detach from a shared memory segment
 * without destroying it.
 * 
 * @param mem pointer to shatomic struct
 */
int shatomic_detach(struct shatomic *mem);

/**
 * @brief Insert atomically value into shared memory segment
 * 
 * Insert atomically a copy of @value into shared memory segment
 * @mem, at index @index.
 * 
 * @param mem pointer to shatomic struct
 * @param index the index to be used
 * @param value the value that must be inserted into the segment
 */
void shatomic_put_value(struct shatomic* mem, int index, int value);

/**
 * @brief Get atomically value from shared memory segment
 * 
 * Get atomically a copy of value at @index from shared memory segment
 * @mem.
 * 
 * @param mem pointer to shatomic struct
 * @param index the index to be used
 * @return value read from memory segment
 */
int shatomic_get_value(struct shatomic* mem, int index);

#endif
