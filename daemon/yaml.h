#include "vector.h"
#include <errno.h>
#include <stdbool.h>
#include <yaml.h>

// ---------------------------------------------------------

#define YAML_GET(name, type)                                                   \
    int yaml_get_##name(const yaml_document_t *document,                       \
        const yaml_node_t *node, type *out_value)

YAML_GET(string, char *);
YAML_GET(long, long);
YAML_GET(bool, bool);

// ---------------------------------------------------------

#define __YAML_HOWMANY_NODES(d, pn, t, f)                                      \
    ((pn)->data.t.f.top - (pn)->data.t.f.start)

#define __YAML_FOREACH_NODE(d, pn, i, t, f)                                    \
    for ((i) = 0; (i) < ((pn)->data.t.f.top - (pn)->data.t.f.start); ++(i))

#define __YAML_NODE(d, pn, i, t, f, ...)                                       \
    (yaml_document_get_node((d), (pn)->data.t.f.start[(i)] __VA_ARGS__))

// -- Sequence

#define YAML_HOWMANY_ITEMS(d, pn, t, f)                                        \
    __YAML_HOWMANY_NODES(d, pn, sequence, items)

#define YAML_FOREACH_ITEM(d, pn, i)                                            \
    __YAML_FOREACH_NODE(documemt, pn, i, sequence, items)

#define YAML_NODE_ITEM(d, pn, i) __YAML_NODE(d, pn, i, sequence, items)

// -- Mapping

#define YAML_FOREACH_PAIR(d, pn, i)                                            \
    __YAML_FOREACH_NODE(d, pn, i, mapping, pairs)

#define YAML_NODE_KEY(d, pn, i) __YAML_NODE(d, pn, i, mapping, pairs, .key)

#define YAML_NODE_VALUE(d, pn, i) __YAML_NODE(d, pn, i, mapping, pairs, .value)

// ---------------------------------------------------------

#define YAML_PARSER_FN(fn_name, ...)                                           \
    int fn_name(yaml_document_t *document, yaml_node_t *node, __VA_ARGS__)

#define YAML_PARSER_FN_TYPE(fn_name, ...)                                      \
    typedef YAML_PARSER_FN((*fn_name), __VA_ARGS__)

YAML_PARSER_FN_TYPE(yaml_parser_fn_t, void *conf);
// typedef int (*yaml_parser_fn_t)(yaml_document_t *document, yaml_node_t *node,
// void *out);

typedef struct yaml_parser_map
{
    const char *key;
    yaml_parser_fn_t parser;
} yaml_parser_map_t;

// ---------------------------------------------------------

// -- Getting a bit too meta on the programming side here

int yaml_parse_mapping(yaml_document_t *document, yaml_node_t *node,
    const yaml_parser_map_t *keys, const size_t keys_size, void *out)
{
    char *key = NULL;
    bool found;
    int i;
    int ret = 0;

    if (node->type != YAML_MAPPING_NODE)
    {
        ret = 1;
        goto end;
    }

    YAML_FOREACH_PAIR (document, node, i)
    {
        free(key);
        key = NULL;

        ret = yaml_get_string(document, YAML_NODE_KEY(document, node, i), &key);
        if (ret)
            goto end;

        found = false;
        for (size_t k = 0; k < keys_size && !found; ++k)
        {
            if (strcmp(keys[k].key, key) == 0)
            {
                found = true;

                ret = (*(keys[k].parser)) (document,
                    YAML_NODE_VALUE(document, node, i), out);

                if (ret)
                    goto end;
            }
        }

        // if (!found)
        // {
        //     ret = 3;
        //     goto end;
        // }
    }

end:
    free(key);
    key = NULL;

    return ret;
}

int yaml_parse_list(yaml_document_t *document, yaml_node_t *node,
    yaml_parser_fn_t parser, vector_t *vector, size_t item_size)
{
    int ret = 0;
    int i;

    if (node->type != YAML_SEQUENCE_NODE)
    {
        ret = 1;
        goto end;
    }

    YAML_FOREACH_ITEM (document, node, i)
    {
        void *new_elem = malloc(vector->item_size);
        if (!new_elem)
        {
            ret = 1;
            goto end;
        }

        ret = (*parser)(document, YAML_NODE_ITEM(document, node, i), new_elem);
        if (ret)
            goto end;

        vector_push_back(vector, new_elem);
        free(new_elem);
        new_elem = NULL;
    }

end:
    return ret;
}

// ---------------------------------------------------------

// NOTE: the string is allocated using dynamic memory, please free after use!
int yaml_get_string(const yaml_document_t *document, const yaml_node_t *node,
    char **output_value)
{
    const char *ptr;
    char *newptr;
    size_t length;
    int ret = 0;

    if (node->type != YAML_SCALAR_NODE)
    {
        ret = 1;
        goto end;
    }

    ptr = (const char *) node->data.scalar.value;
    length = node->data.scalar.length;

    newptr = (char *) calloc(length + 1, sizeof(char));
    if (!newptr)
    {
        ret = 2;
        goto end;
    }

    strncpy(newptr, ptr, length);
    *output_value = newptr;

end:
    return ret;
}

int string_to_long(char *strvalue, long *out)
{
    char *endptr;
    long value;
    int ret = 0;
    value = strtol(strvalue, &endptr, 10);
    if (errno != 0)
    {
        ret = 2;
        goto end;
    }

    if (endptr == strvalue)
    {
        // No characters were consumed!
        ret = 3;
        goto end;
    }

    *out = value;

end:
    return ret;
}

int string_to_bool(char *strvalue, bool *out)
{
    int ret = 0;

    if (strcmp(strvalue, "true") == 0)
        *out = true;
    else if (strcmp(strvalue, "false") == 0)
        *out = false;
    else
        ret = 2;

    return ret;
}

int yaml_get_long(const yaml_document_t *document, const yaml_node_t *node,
    long *output_value)
{
    char *strvalue = NULL;
    int ret = 0;

    ret = yaml_get_string(document, node, &strvalue);
    if (ret)
        goto end;

    ret = string_to_long(strvalue, output_value);
    if (ret)
        goto end;

end:
    free(strvalue);
    return ret;
}

int yaml_get_bool(const yaml_document_t *document, const yaml_node_t *node,
    bool *output_value)
{
    char *strvalue = NULL;
    int ret = 0;

    ret = yaml_get_string(document, node, &strvalue);
    if (ret)
        goto end;

    ret = string_to_bool(strvalue, output_value);
    if (ret)
        goto end;

end:
    free(strvalue);
    return ret;
}

// ---------------------------------------------------------
