/**
 * @file shatomic.c
 * @author Gabriele Serra
 * @date 10 Nov 2018
 * @brief Contains the implementation of a pseudo-atomic shared memory segment 
 */

#include "shatomic.h"
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// ---------------------------------------------
// PRIVATE METHODS
// ---------------------------------------------

/**
 * @internal
 *
 * Generates a unique temporary filename from template, 
 * creates and opens the file. After generate a key using
 * i-node of the file. Finally removes the file.
 * 
 * @endinternal
 */
static int createkey() {
    key_t key;
    char template[KEY_LEN];

    strncpy(template, KEY_TEMPLATE, KEY_LEN);

    if(mkstemp(template) < 0)
        return -1;
    
    key = ftok(template, 0);
    
    if (key < 0)
        return -1;

    unlink(template);
    return key;
}

// ---------------------------------------------
// PUBLIC METHODS
// ---------------------------------------------

/**
 * @internal
 *
 * Initialize the shatomic structure filling with 0s
 * 
 * @endinternal
 */
void shatomic_init(struct shatomic* mem) {
    mem->id = 0;
    mem->key = 0;
    mem->value = NULL;
}

/**
 * @internal
 * 
 * Generate the memory segment key and allocate
 * a shared atomic memory segment of @nvalue number
 * of value.
 * 
 * @endinternal
 */
int shatomic_create(struct shatomic* mem, int nvalue) {
    key_t key;
    int shmid;
    size_t size;
    
    key = createkey();

    if(key < 0)
        return -1;

    mem->nvalue = nvalue;
    size = nvalue * sizeof(atomic_t);
    shmid = shmget(key, size, PRIVILEGES | IPC_CREAT);
    
    if (shmid < 0)
        return -1;

    mem->key = key;
    mem->id = shmid;
    return 0;
}

/**
 * @internal Read and return the key of an existent shared memory segment
 * 
 * Read and return the key of an existent shared memory segment.
 * 
 * @endinternal
 */
key_t shatomic_getkey(struct shatomic* mem) {
    return mem->key;
}

/**
 * @internal Get an existent memory segment
 * 
 * Get an existent memory segment that have key @key
 * and @num as number of element.
 * 
 * @endinternal
 */
int shatomic_use(struct shatomic* mem, key_t key, int num) {
    int shmid;
    
    shmid = shmget(key, sizeof(atomic_t), PRIVILEGES);
    
    if (shmid < 0)
        return -1;
    
    mem->key = key;
    mem->id = shmid;
    mem->nvalue = num;
    return 0;
}

/**
 * @internal
 * 
 * Attaches to a previous created shared memory segment. Memory
 * segment must be previous created with shatomic_create(...) or
 * get with shatomic_use(...)
 * 
 * @endinternal
 */
int shatomic_attach(struct shatomic* mem) {
    atomic_t* value;
    
    value = shmat(mem->id, NULL, 0);
    
    if(mem->value < (atomic_t*) 0)
        return -1;
    
    mem->value = value;
    return 0;
}

/**
 * @internal
 * 
 * Removes a shared memory segment and deallocate
 * memory to free space.
 * 
 * @endinternal
 */
int shatomic_destroy(struct shatomic* mem) {
    return shmctl(mem->id, IPC_RMID, NULL);
}

int shatomic_detach(struct shatomic *mem) {
    return shmdt(mem->value);
}

void shatomic_put_value(struct shatomic* mem, int index, int value) {
    if(index >= mem->nvalue)
        return;
    
    atomic_set(mem->value+(index * sizeof(atomic_t)), value);
}

int shatomic_get_value(struct shatomic* mem, int index) {
    return atomic_read(mem->value+(index * sizeof(atomic_t)));
}
