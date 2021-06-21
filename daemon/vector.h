#include <stdbool.h>
#include <stdlib.h>

#define __VECTOR_CONTENT(type)                                                 \
    {                                                                          \
        size_t capacity;                                                       \
        size_t item_size;                                                      \
        size_t size;                                                           \
        type *data;                                                            \
    }

// Use this to declare a with elements of type type
#define VECTOR(type) struct __VECTOR_CONTENT(type)

// Generic vector type
typedef struct vector __VECTOR_CONTENT(void) vector_t;

// Use this to automatically get the size of the elements of
// a vector (or pointer to vector)
#define VECTOR_ISIZE(v) (sizeof(*((v).data)))
#define VECTOR_P_ISIZE(v) VECTOR_ISIZE(*(v))

#define VECTOR_FOREACH(v, p)                                                   \
    for ((p) = &((v).data[0]); p != &((v).data[(v).size]); ++p)
#define VECTOR_P_FOREACH(v, p) VECTOR_FOREACH (*(v), p)

// ========================================================================== //
// ------------------------- Function Declarations -------------------------- //
// ========================================================================== //

// Initializes a vector (be careful! you must provide the
// size of its items!)
void vector_initialize(vector_t *v, size_t item_size);
static inline void vector_swap(vector_t *v1, vector_t *v2);

static inline size_t __vector_addr_diff(void *first, void *last);
static inline void *__vector_addr_add(void *first, size_t diff);

// All these functions return pointer to elements (end returns an oob pointer
// just past the last element)
static inline void *vector_at(vector_t *v, size_t i);
static inline void *vector_front(vector_t *v);
static inline void *vector_back(vector_t *v);
static inline void *vector_begin(vector_t *v);
static inline void *vector_end(vector_t *v);
static inline void *vector_data(vector_t *v);

// Getters
static inline bool vector_empty(vector_t *v);
static inline size_t vector_size(vector_t *v);
static inline size_t vector_capacity(vector_t *v);

// Size/capacity modifiers
int vector_resize(vector_t *v, size_t des_size);
int vector_reserve(vector_t *v, size_t new_cap);
int vector_shrink_to_fit(vector_t *v);
static inline void vector_clear(vector_t *v);

// Insert operations
static inline int vector_insert(vector_t *v, size_t pos, void *value);
int vector_insert_ncopies(vector_t *v, size_t pos, size_t count, void *value);
int vector_insert_multi(vector_t *v, size_t pos, void *first, void *last);

// Erase operations
static inline int vector_erase(vector_t *v, size_t pos);
int vector_erase_multi(vector_t *v, size_t first, size_t last);

// Push and pop
static inline int vector_push_back(vector_t *v, void *value);
int vector_pop_back(vector_t *v, void *value);

// ========================================================================== //
// ------------------- Static Inline Function Definitions ------------------- //
// ========================================================================== //

static inline size_t __vector_addr_diff(void *first, void *last)
{
    typedef char byte_t;
    return ((byte_t *) last) - ((byte_t *) first);
}

static inline void *__vector_addr_add(void *first, size_t diff)
{
    typedef char byte_t;
    return ((byte_t *) first) + diff;
}

static inline void *vector_at(vector_t *v, size_t i)
{
    return __vector_addr_add(v->data, i * v->item_size);
}

static inline void *vector_front(vector_t *v)
{
    return vector_at(v, 0);
}

static inline void *vector_back(vector_t *v)
{
    return vector_at(v, v->size - 1);
}

static inline void *vector_begin(vector_t *v)
{
    return vector_at(v, 0);
}

static inline void *vector_end(vector_t *v)
{
    return vector_at(v, v->size);
}

static inline void *vector_data(vector_t *v)
{
    return v->data;
}

static inline bool vector_empty(vector_t *v)
{
    return v->size < 1;
}

static inline size_t vector_size(vector_t *v)
{
    return v->size;
}

static inline size_t vector_capacity(vector_t *v)
{
    return v->capacity;
}

// Structures inside are implicitly invalidated
static inline void vector_clear(vector_t *v)
{
    vector_resize(v, 0);
}

static inline int vector_insert(vector_t *v, size_t pos, void *value)
{
    return vector_insert_multi(v, pos, value,
        __vector_addr_add(value, v->item_size));
}

static inline int vector_erase(vector_t *v, size_t pos)
{
    return vector_erase_multi(v, pos, pos + 1);
}

static inline int vector_push_back(vector_t *v, void *value)
{
    return vector_insert(v, vector_size(v), value);
}

static inline void vector_swap(vector_t *v1, vector_t *v2)
{
    vector_t v;
    v = *v1;
    *v1 = *v2;
    *v2 = v;
}
