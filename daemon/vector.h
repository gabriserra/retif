#include <stdlib.h>
#include <malloc.h>
#include <string.h>

typedef struct vector
{
    size_t size;
    void *at;
} vector_t;

#define VECTOR_TYPE(type) \
    struct                \
    {                     \
        size_t size;      \
        type *at;         \
    }

#define VECTOR(type, name) \
    VECTOR_TYPE(type)      \
    name

void vector_initialize(vector_t *v)
{
    v->size = 0;
    v->at = NULL;
}

void *vector_at(vector_t *v, size_t i, size_t size)
{
    typedef char byte_t;
    return (((byte_t *)v->at) + (i * size));
}

int vector_realloc(vector_t *v, size_t nmemb, size_t size)
{
    int res = 0;
    vector_t old = *v;

    v->size = nmemb;
    v->at = reallocarray(v->at, nmemb, size);

    if (v->at == NULL) {
        *v = old;
        res = 1;
        goto end;
    }

    if (v->size > old.size)
    {
        size_t diff = v->size - old.size;
        memset(vector_at(v, old.size, size), 0, size * (diff));
    }

end:
    return res;
}

// #define VECTOR_INITIALIZER \
//     {                      \
//         0                  \
//     }

// #define VECTOR_INITIALIZE(v_ptr) \
//     memset(v_ptr, 0, sizeof(*v_ptr))

// #define __VECTOR_P_SIZE(v_ptr) \
//     ((v_ptr)->size)

// #define __VECTOR_P_PTR(v_ptr) \
//     ((v_ptr)->at)

// #define __VECTOR_SET_SIZE(v_ptr, newsize) \
//     (__VECTOR_P_SIZE(v_ptr) = (newsize))

// #define __VECTOR_REALLOC_TYPE(at, newsize, type) \
//     ((type *)(reallocarray((at), (newsize), sizeof(type))))

// #define __VECTOR_REALLOC_SIZE(at, newsize, size_type) \
//     ((reallocarray((at), (newsize), size_type)))

// #define __VECTOR_JUMP_AT_ELEM(v_ptr, i, size_type) \
//     (((char*)(__VECTOR_P_PTR(v_ptr)) + (i * size_type))

// // TODO: error checking
// #define VECTOR_REALLOC(v_ptr, newsize, type)                            \
//     (__VECTOR_SET_SIZE(v_ptr, newsize),                                 \
//      (__VECTOR_P_PTR(v_ptr) =                                           \
//           __VECTOR_REALLOC_TYPE(__VECTOR_P_PTR(v_ptr), newsize, type)), \
//      (memset(__VECTOR_P_PTR(v_ptr) + ((newsize)-1), 0, sizeof(type))))

// #define VECTOR_FREE(v_ptr)            \
//     do                                \
//     {                                 \
//         free(__VECTOR_P_PTR(v_ptr));  \
//         __VECTOR_P_PTR(v_ptr) = NULL; \
//     } while (0)

// #define VECTOR_AT(vector, i) \
//     ((vector).at[(i)])

// #define VECTOR_AT_SIZE_PTR(vector, i, size_type) \
//     (((char *)((vector)->at)) + (i * size_type))

// #define VECTOR_REALLOC_SIZE(v_ptr, newsize, size_type)                       \
//     (__VECTOR_SET_SIZE(v_ptr, newsize),                                      \
//      (__VECTOR_P_PTR(v_ptr) =                                                \
//           __VECTOR_REALLOC_SIZE(__VECTOR_P_PTR(v_ptr), newsize, size_type)), \
//      (memset(VECTOR_AT_SIZE_PTR(v_ptr, (newsize)-1, size_type), 0, size_type)))
