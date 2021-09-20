#include "vector.h"

#include <malloc.h>
#include <string.h>

const vector_t VECTOR_INITIALIZER = {0};

void vector_initialize(vector_t *v, size_t item_size)
{
    *v = VECTOR_INITIALIZER;
    v->item_size = item_size;
}

// Can both increase or shrink the capacity of a vector, without ever affecting
// already existing elements (if can_erase is false and the resulting capacity
// is less than its size, an error is returned and no operation is performed)
int __vector_realloc(vector_t *v, size_t new_cap, bool can_erase)
{
    void *new_vec;

    if (v->capacity == new_cap)
        return 0;

    if (!can_erase && v->size > new_cap)
        return 1;

    if (!(new_vec = reallocarray(v->data, new_cap, v->item_size)))
        return 1;

    v->data = new_vec;

    v->capacity = new_cap;
    v->size = v->size > v->capacity ? v->capacity : v->size;
    return 0;
}

// Changes the capacity of the vector so that it can contain
// at least new_cap elements in total
int vector_reserve(vector_t *v, size_t new_cap)
{
    if (new_cap <= v->capacity)
        return 0;

    // Aggressive reallocation first: try requesting more memory than needed to
    // reduce the number of reallocations
    size_t overalloc = v->capacity * 2;
    if (new_cap < overalloc)
    {
        if (!__vector_realloc(v, overalloc, false))
            return 0;
    }

    // Last chance: request exactly the memory needed
    return __vector_realloc(v, new_cap, false);
}

int vector_shrink_to_fit(vector_t *v)
{
    return __vector_realloc(v, v->size, false);
}

// Insert count copies of the specified value before v[pos].
int vector_insert_ncopies(vector_t *v, size_t pos, size_t count, void *value)
{
    int res = 0;
    for (size_t i = 0; i < count && !res; ++i)
    {
        res = vector_insert(v, pos, value);
    }
    return res;
}

// size_t vector_iter_diff(void *first, void *last, size_t item_size)
// {
// return __vector_addr_diff(first, last) / item_size;
// }

// Insert the content between [first, last), if pos_addr is
// a valid address inside this vector. NOTE: assumes that
// alignment of all addresses is valid for this array!
int vector_insert_multi(vector_t *v, size_t pos, void *first, void *last)
{
    // Empty set does nothing
    if (first == last)
        return 0;

    // Order check
    if (first > last)
        return 1;

    // No out of bounds assignments
    if (pos > vector_size(v))
        return 1;

    size_t addr_diff = __vector_addr_diff(first, last);
    size_t des_size = v->size + (addr_diff / v->item_size);

    // Overflow check
    if (des_size < v->size)
        return 1;

    // Capacity check
    if (vector_reserve(v, des_size))
        return 1;

    // Push forward all elements between pos and the end of
    // the vector onward of diff elements exactly
    void *pos_offset = vector_at(v, pos);
    void *forward_dest = __vector_addr_add(pos_offset, addr_diff);
    memmove(forward_dest, pos_offset,
        __vector_addr_diff(vector_end(v), pos_offset));

    // Copy incoming values into the newly freed space
    memmove(pos_offset, first, addr_diff);

    // Update info
    v->size = des_size;
    return 0;
}

int vector_erase_multi(vector_t *v, size_t first, size_t last)
{
    // Empty set does nothing
    if (first == last)
        return 0;

    // Order check
    if (first > last)
        return 1;

    // Bounds check
    if (last > vector_size(v))
        return 1;

    // Just move back elements past last until vector_end ...
    size_t addr_diff = __vector_addr_diff(vector_at(v, last), vector_end(v));
    memmove(vector_at(v, first), vector_at(v, last), addr_diff);

    // ... and reduce vector size
    v->size = v->size - (last - first);
    return 0;
}

int vector_pop_back(vector_t *v, void *value)
{
    if (v->size < 1)
        return 1;
    memmove(value, vector_back(v), v->item_size);
    return vector_erase(v, v->size - 1);
}

int vector_resize(vector_t *v, size_t des_size)
{
    if (vector_reserve(v, des_size))
        return 1;

    if (v->size < des_size)
    {
        // Initialize with zeroes
        memset(vector_end(v), 0, (des_size - v->size) * v->item_size);
    }

    v->size = des_size;
    return 0;
}
