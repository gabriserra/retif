#include "vector.h"
#include <errno.h>
#include <stdbool.h>
#include <yaml.h>

// ---------------------------------------------------------

#define YAML_GET(name, type) \
    int yaml_get_##name(const yaml_document_t *document, const yaml_node_t *node, type *out_value)

YAML_GET(string, char *);
YAML_GET(long, long);
YAML_GET(bool, bool);

// ---------------------------------------------------------

#define __YAML_HOWMANY_NODES(d, pn, t, f) \
    ((pn)->data.t.f.top - (pn)->data.t.f.start)

#define __YAML_FOREACH_NODE(d, pn, i, t, f) \
    for ((i) = 0; (i) < ((pn)->data.t.f.top - (pn)->data.t.f.start); ++(i))

#define __YAML_NODE(d, pn, i, t, f, ...) \
    (yaml_document_get_node((d), (pn)->data.t.f.start[(i)] __VA_ARGS__))

// -- Sequence

#define YAML_HOWMANY_ITEMS(d, pn, t, f) \
    __YAML_HOWMANY_NODES(d, pn, sequence, items)

#define YAML_FOREACH_ITEM(d, pn, i) \
    __YAML_FOREACH_NODE(documemt, pn, i, sequence, items)

#define YAML_NODE_ITEM(d, pn, i) \
    __YAML_NODE(d, pn, i, sequence, items)

// -- Mapping

#define YAML_FOREACH_PAIR(d, pn, i) \
    __YAML_FOREACH_NODE(d, pn, i, mapping, pairs)

#define YAML_NODE_KEY(d, pn, i) \
    __YAML_NODE(d, pn, i, mapping, pairs, .key)

#define YAML_NODE_VALUE(d, pn, i) \
    __YAML_NODE(d, pn, i, mapping, pairs, .value)

// ---------------------------------------------------------

#define YAML_PARSER_FN(fn_name, ...) \
    int fn_name(yaml_document_t *document, yaml_node_t *node, __VA_ARGS__)

#define YAML_PARSER_FN_TYPE(fn_name, ...) \
    typedef YAML_PARSER_FN((*fn_name), __VA_ARGS__)

YAML_PARSER_FN_TYPE(yaml_parser_fn_t, void *conf);
// typedef int (*yaml_parser_fn_t)(yaml_document_t *document, yaml_node_t *node, void *out);

typedef struct yaml_parser_map
{
    const char *key;
    yaml_parser_fn_t parser;
} yaml_parser_map_t;

// ---------------------------------------------------------

// -- Getting a bit too meta on the programming side here

int yaml_parse_mapping(
    yaml_document_t *document,
    yaml_node_t *node,
    const yaml_parser_map_t *keys,
    const size_t keys_size,
    void *out)
{
    char *key = NULL;
    bool found;
    int i;
    int ret = 0;

    if (node->type != YAML_MAPPING_NODE)
    {
        ret = 1;
        goto _end;
    }

    YAML_FOREACH_PAIR(document, node, i)
    {
        free(key);
        key = NULL;

        ret = yaml_get_string(document, YAML_NODE_KEY(document, node, i), &key);
        if (ret)
            goto _end;

        found = false;
        for (size_t k = 0; k < keys_size && !found; ++k)
        {
            if (strcmp(keys[k].key, key) == 0)
            {
                found = true;

                ret =
                    (*(keys[k].parser))(document, YAML_NODE_VALUE(document, node, i), out);

                if (ret)
                    goto _end;
            }
        }

        if (!found)
        {
            ret = 3;
            goto _end;
        }
    }

_end:
    free(key);
    key = NULL;

    return ret;
}

int yaml_parse_list(yaml_document_t *document, yaml_node_t *node, yaml_parser_fn_t parser, vector_t *vector, size_t item_size)
{
    int ret = 0;
    int i;

    if (node->type != YAML_SEQUENCE_NODE)
    {
        ret = 1;
        goto _end;
    }

    vector_initialize(vector);

    YAML_FOREACH_ITEM(document, node, i)
    {
        vector_realloc(vector, i + 1, item_size);
        ret = (*parser)(document, YAML_NODE_ITEM(document, node, i),
                        vector_at(vector, i, item_size));
        if (ret)
            goto _end;
    }

_end:
    return ret;
}

// #define YAML_PARSE_LIST(document, node, parser, vector_type, out_vector_ptr, retv)                             \
//     {                                                                                                          \
//         int _i;                                                                                                \
//         VECTOR_INITIALIZE(out_vector_ptr);                                                                     \
//         YAML_FOREACH_ITEM(document, node, _i)                                                                  \
//         {                                                                                                      \
//             VECTOR_REALLOC(out_vector_ptr, (_i + 1), vector_type);                                             \
//             retv = (*parser)(document, YAML_NODE_ITEM(document, node, _i), &VECTOR_AT(*(out_vector_ptr), _i)); \
//             if (retv)                                                                                          \
//                 goto _end;                                                                                     \
//         }                                                                                                      \
//     _end:;                                                                                                     \
//         ret = retv;                                                                                        \
//     }

// // WARNING: NEVER USE EXPRESSIONS AS ARGUMENTS OF THIS MACRO! ALWAYS USE REAL VARIABLE NAMES!
// #define YAML_PARSE_MAPPING(document, node, keys, parsers, out, retv)                         \
//     {                                                                                        \
//         char *_key;                                                                          \
//         bool _found;                                                                         \
//         int _i;                                                                              \
//         if (node->type != YAML_MAPPING_NODE)                                                 \
//         {                                                                                    \
//             retv = 1;                                                                        \
//             goto _end;                                                                       \
//         }                                                                                    \
//                                                                                              \
//         YAML_FOREACH_PAIR(document, node, _i)                                                \
//         {                                                                                    \
//             retv = yaml_get_string(document, YAML_NODE_KEY(document, node, _i), &_key);      \
//             if (retv)                                                                        \
//             {                                                                                \
//                 goto _end;                                                                   \
//             }                                                                                \
//                                                                                              \
//             _found = false;                                                                  \
//             printf("Key: %s...\n", _key);                                                    \
//             for (size_t k = 0; k < keys_len && !_found; ++k)                                 \
//             {                                                                                \
//                 if (strcmp(keys[k], _key) == 0)                                              \
//                 {                                                                            \
//                     printf("Found! Going down...\n");                                        \
//                     _found = true;                                                           \
//                     retv =                                                                   \
//                         (*(parsers[k]))(document, YAML_NODE_VALUE(document, node, _i), out); \
//                     if (retv)                                                                \
//                         goto _end;                                                           \
//                     break;                                                                   \
//                 }                                                                            \
//             }                                                                                \
//                                                                                              \
//             if (!_found)                                                                     \
//             {                                                                                \
//                 retv = 3;                                                                    \
//                 goto _end;                                                                   \
//             }                                                                                \
//         }                                                                                    \
//     _end:                                                                                    \
//         free(_key);                                                                          \
//         _key = NULL;                                                                         \
//     }

// ---------------------------------------------------------

// NOTE: the string is allocated using dynamic memory, please free after use!
int yaml_get_string(const yaml_document_t *document, const yaml_node_t *node, char **output_value)
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

    ptr = (const char *)node->data.scalar.value;
    length = node->data.scalar.length;

    newptr = (char *)calloc(length + 1, sizeof(char));
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

int string_to_bool(char *strvalue, bool*out) {
    int ret = 0;

    if (strcmp(strvalue, "true") == 0)
        *out = true;
    else if (strcmp(strvalue, "false") == 0)
        *out = false;
    else
        ret = 2;

    return ret;
}

int yaml_get_long(const yaml_document_t *document, const yaml_node_t *node, long *output_value)
{
    char *strvalue = NULL;
    char *endptr;
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

int yaml_get_bool(const yaml_document_t *document, const yaml_node_t *node, bool *output_value)
{
    char *strvalue = NULL;
    bool value;
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

// #define DECLARE_PARSER_FN(fn_name, ...) \
//     int fn_name(yaml_document_t *document, yaml_node_t *node, __VA_ARGS__)

// #define DECLARE_PARSER_FN_TYPE(fn_name, ...) \
//     typedef int (*fn_name)(yaml_document_t * document, yaml_node_t * node, __VA_ARGS__)

// DECLARE_PARSER_FN(parse_conf_kernel, struct configuration *out);
// DECLARE_PARSER_FN(parse_conf_plugins, struct configuration *out);
// DECLARE_PARSER_FN(parse_conf_rules, struct configuration *out);

// DECLARE_PARSER_FN(parse_conf_kernel_rr_timeslice, struct configuration *out);
// DECLARE_PARSER_FN(parse_conf_kernel_rt_period, struct configuration *out);
// DECLARE_PARSER_FN(parse_conf_kernel_rt_runtime, struct configuration *out);

// DECLARE_PARSER_FN(parse_conf_kernel_rr_timeslice, struct configuration *out)
// {
//     return yaml_get_long(document, node, &out->kernel.rr_timeslice);
// }

// DECLARE_PARSER_FN(parse_conf_kernel_sched_rt_period, struct configuration *out)
// {
//     return yaml_get_long(document, node, &out->kernel.sched_rt_period);
// }

// DECLARE_PARSER_FN(parse_conf_kernel_sched_rt_runtime, struct configuration *out)
// {
//     return yaml_get_long(document, node, &out->kernel.sched_rt_runtime);
// }

// int parse_conf_kernel(yaml_document_t *document, yaml_node_t *node, struct configuration *out)
// {
//     const char *keys[] = {
//         "rr_timeslice",
//         "sched_rt_period",
//         "sched_rt_runtime",
//     };
//     const parse_fun_t parsers[] = {
//         parse_conf_kernel_rr_timeslice,
//         parse_conf_kernel_sched_rt_period,
//         parse_conf_kernel_sched_rt_runtime,
//     };
//     const size_t keys_len = sizeof(keys) / sizeof(const char *);
//     int ret = 0;
//     YAML_PARSE_MAPPING(document, node, keys, parsers, out, ret);
//     return ret;
// }

// DECLARE_PARSER_FN(parse_conf_plugins_item_name, conf_plugin_t *out)
// {
//     printf("HELLO THERE\n");
//     return yaml_get_string(document, node, &out->name);
// }

// DECLARE_PARSER_FN(parse_conf_plugins_item_plugin, conf_plugin_t *out)
// {
//     return yaml_get_string(document, node, &out->plugin_path);
// }
// DECLARE_PARSER_FN(parse_conf_plugins_item_priority, conf_plugin_t *out)
// {
//     // TODO: could be a list of values or a special string that denotes min and max value
//     return 0;
// }
// DECLARE_PARSER_FN(parse_conf_plugins_item_cores, conf_plugin_t *out)
// {
//     // TODO: could be a list of values or special string that denotes a list
//     return 0;
// }

// // int parse_conf_plugins_item(yaml_document_t *document, yaml_node_t *node, struct conf_plugin *out_item)
// // {
// //     const char *keys[] = {
// //         "name",
// //         "plugin",
// //         "priority",
// //         "cores",
// //     };
// //     const parse_fun_t parsers[] = {
// //         parse_conf_plugins_item_name,
// //         parse_conf_plugins_item_plugin,
// //         parse_conf_plugins_item_priority,
// //         parse_conf_plugins_item_cores,
// //     };
// //     const size_t keys_len = sizeof(keys) / sizeof(const char *);
// //     int ret = 0;
// //     YAML_PARSE_MAPPING(document, node, keys, parsers, out, ret);
// //     return ret;
// // }

// DECLARE_PARSER_FN(parse_conf_plugins_node, conf_plugin_t *item)
// {
//     const char *keys[] = {
//         "name",
//         "plugin",
//         "priority",
//         "cores",
//     };

//     DECLARE_PARSER_FN_TYPE(parse_fun_item_t, conf_plugin_t * item);

//     const parse_fun_item_t parsers[] = {
//         parse_conf_plugins_item_name,
//         parse_conf_plugins_item_plugin,
//         parse_conf_plugins_item_priority,
//         parse_conf_plugins_item_cores,
//     };
//     const size_t keys_len = sizeof(keys) / sizeof(const char *); // TODO: keys_len in macro
//     int ret = 0;
//     YAML_PARSE_MAPPING(document, node, keys, parsers, item, ret);
//     return ret;
// }

// #define YAML_PARSE_LIST(document, node, parser, vector_type, out_vector_ptr, retv)                             \
//     {                                                                                                          \
//         int _i;                                                                                                \
//         VECTOR_INITIALIZE(out_vector_ptr);                                                                     \
//         YAML_FOREACH_ITEM(document, node, _i)                                                                  \
//         {                                                                                                      \
//             VECTOR_REALLOC(out_vector_ptr, (_i + 1), vector_type);                                             \
//             retv = (*parser)(document, YAML_NODE_ITEM(document, node, _i), &VECTOR_AT(*(out_vector_ptr), _i)); \
//             if (retv)                                                                                          \
//                 goto _end;                                                                                     \
//         }                                                                                                      \
//     _end:;                                                                                                     \
//         ret = retv;                                                                                        \
//     }

// int parse_conf_plugins(yaml_document_t *document, yaml_node_t *node, struct configuration *out)
// {
//     int ret = 0;
//     YAML_PARSE_LIST(document, node, parse_conf_plugins_node, conf_plugin_t, &out->plugins, ret)
//     return ret;
// }

// // int parse_conf_plugins(yaml_document_t *document, yaml_node_t *node, struct configuration *out_conf)
// // {
// //     int ret = 0;
// //     VECTOR_INITIALIZE(&out_conf->plugins);
// //     {
// //         int retv = ret;
// //         int _i;
// //         YAML_FOREACH_ITEM(document, node, _i)
// //         {
// //             VECTOR_REALLOC(&out_conf->plugins, (_i + 1), conf_plugin_t);
// //             retv = (*parse_conf_plugins_node)(document, YAML_NODE_ITEM(document, node, _i), &VECTOR_AT(out_conf->plugins, _i));
// //             if (retv)
// //                 goto _end;
// //         }
// //     _end:;
// //         ret = retv;
// //     }
// //     return ret;
// // }

// int parse_conf_rules(yaml_document_t *document, yaml_node_t *node, struct configuration *out)
// {
//     printf("Parsing %s configuration, node content:\n", "rules");
//     print_node(document, node);
//     printf("\n\n");
//     return 0;
// }

// int parse_configuration(yaml_document_t *document, yaml_node_t *node, struct configuration *out)
// {
//     const char *keys[] = {
//         "configuration",
//         "kernel",
//         "plugins",
//         "rules",
//     };
//     const parse_fun_t parsers[] = {
//         parse_configuration,
//         parse_conf_kernel,
//         parse_conf_plugins,
//         parse_conf_rules,
//     };
//     const size_t keys_len = sizeof(keys) / sizeof(const char *);
//     int ret = 0;
//     YAML_PARSE_MAPPING(document, node, keys, parsers, out, ret);
//     return ret;
// }

// int print_node(yaml_document_t *document, yaml_node_t *node)
// {
//     yaml_char_t *value;
//     yaml_node_item_t *item;
//     yaml_node_t *next_node;
//     int i;
//     int k;
//     int maxk;

//     switch (node->type)
//     {
//     case YAML_SCALAR_NODE:
//         value = node->data.scalar.value;
//         printf("\"%s\"", value);
//         break;
//     case YAML_SEQUENCE_NODE:
//         printf("[\n");
//         maxk = node->data.sequence.items.top - node->data.sequence.items.start;
//         for (k = 0; k < maxk; ++k)
//         {
//             next_node = yaml_document_get_node(document, node->data.sequence.items.start[k]);
//             print_node(document, next_node);
//             if (k < maxk - 1)
//                 printf(",");
//         }
//         printf("]\n");
//         break;
//     case YAML_MAPPING_NODE:
//         printf("{\n");
//         maxk = node->data.mapping.pairs.top - node->data.mapping.pairs.start;
//         for (k = 0; k < maxk; ++k)
//         {
//             // key
//             next_node = yaml_document_get_node(document, node->data.mapping.pairs.start[k].key);
//             print_node(document, next_node);
//             printf(":");
//             // value
//             next_node = yaml_document_get_node(document, node->data.mapping.pairs.start[k].value);
//             print_node(document, next_node);
//             if (k < maxk - 1)
//                 printf(",\n");
//         }
//         printf("}\n");
//         break;
//     }
// }

// int parse_document(const char path[])
// {
//     bool done;
//     int ret = 0;

//     yaml_parser_t parser;
//     yaml_document_t document;

//     yaml_parser_initialize(&parser);

//     FILE *input = fopen(path, "rb");
//     yaml_parser_set_input_file(&parser, input);

//     configuration_t configuration;
//     yaml_node_t *node;

//     int res = yaml_parser_load(&parser, &document);

//     if (res != 1)
//     {
//         ret = 1;
//         goto end;
//     }

//     node = yaml_document_get_root_node(&document);
//     // print_node(&document, node);
//     parse_configuration(&document, node, &configuration);
//     printf("-------------\n");
//     printf("kernel.rr_timeslice: %ld\n", configuration.kernel.rr_timeslice);
//     printf("kernel.sched_rt_period: %ld\n", configuration.kernel.sched_rt_period);
//     printf("kernel.sched_rt_runtime: %ld\n", configuration.kernel.sched_rt_runtime);
// end:
//     yaml_document_delete(&document);
//     yaml_parser_delete(&parser);

//     return ret;
// }

// int main(int argc, char *argv[])
// {
//     return parse_document(argv[1]);
// }
