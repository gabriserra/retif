#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "yaml.h"

// ====================================================== //
// ------------------ Data Structures ------------------- //
// ====================================================== //

typedef struct conf_kernel
{
    long rr_timeslice;     // ms
    long sched_rt_period;  // us
    long sched_rt_runtime; // us
} conf_kernel_t;

typedef struct conf_plugin
{
    char *name;
    char *plugin_path;
    int priority_min;
    int priority_max;
    VECTOR(int, cores);
} conf_plugin_t;

typedef struct acl_properties
{
    double max_utilization;
    long max_runtime;
    long min_period;
    long max_period;
    long min_deadline;
    long max_deadline;
    long min_priority;
    long max_priority;
    bool ignore_adm_test;
} acl_properties_t;

typedef enum acl_domain_kind
{
    ACL_DOMAIN_USER = 0,
    ACL_DOMAIN_GROUP,
    ACL_DOMAIN_NUMBER,
} domain_kind_t;

typedef struct conf_acl_rule
{
    char *domain;
    char *type; // TYPE
    char *plugin_name;
    acl_properties_t properties;
} conf_acl_rule_t;

typedef struct configuration
{
    conf_kernel_t kernel;
    VECTOR(conf_plugin_t, plugins);
    VECTOR(conf_acl_rule_t, acl_rules);
} configuration_t;

// ====================================================== //
// --------------- Function Declarations ---------------- //
// ====================================================== //

YAML_PARSER_FN(parse_conf, configuration_t *out);
YAML_PARSER_FN(parse_conf_kernel, configuration_t *out);
YAML_PARSER_FN(parse_conf_plugins, configuration_t *out);
YAML_PARSER_FN(parse_conf_acl_rules, configuration_t *out);

YAML_PARSER_FN(parse_conf_kernel_rr_timeslice, conf_kernel_t *out);
YAML_PARSER_FN(parse_conf_kernel_sched_rt_period, conf_kernel_t *out);
YAML_PARSER_FN(parse_conf_kernel_sched_rt_runtime, conf_kernel_t *out);

YAML_PARSER_FN(parse_conf_plugins_item, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_name, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_plugin_path, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_priority, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_cores, conf_plugin_t *out);

// ====================================================== //
// ---------------- Function Definitions ---------------- //
// ====================================================== //

const char key_conf[] = "configuration";
const char key_kernel[] = "kernel";
const char key_plugins[] = "plugins";
const char key_acl_rules[] = "rules";

const char key_rr_timeslice[] = "rr_timeslice";
const char key_sched_rt_period[] = "sched_rt_period";
const char key_sched_rt_runtime[] = "sched_rt_runtime";

const char key_name[] = "name";
const char key_plugin[] = "plugin";
const char key_priority[] = "priority";
const char key_cores[] = "cores";

const char key_domain[] = "domain";
const char key_type[] = "type";
const char key_properties[] = "properties";

// TODO: ACL properties

#define YAML_PARSER_MAP_PAIR(key, ptr) \
    {                                  \
        key, (yaml_parser_fn_t)ptr     \
    }

YAML_PARSER_FN(parse_conf, configuration_t *out)
{
    const yaml_parser_map_t map[] = {
        YAML_PARSER_MAP_PAIR(key_conf, parse_conf),
        YAML_PARSER_MAP_PAIR(key_kernel, parse_conf_kernel),
        YAML_PARSER_MAP_PAIR(key_plugins, parse_conf_plugins),
        YAML_PARSER_MAP_PAIR(key_acl_rules, parse_conf_acl_rules),
    };
    const size_t map_size = sizeof(map) / sizeof(yaml_parser_map_t);
    return yaml_parse_mapping(document, node, map, map_size, out);
}

YAML_PARSER_FN(parse_conf_kernel, configuration_t *out)
{
    const yaml_parser_map_t map[] = {
        YAML_PARSER_MAP_PAIR(key_rr_timeslice, parse_conf_kernel_rr_timeslice),
        YAML_PARSER_MAP_PAIR(key_sched_rt_period, parse_conf_kernel_sched_rt_period),
        YAML_PARSER_MAP_PAIR(key_sched_rt_runtime, parse_conf_kernel_sched_rt_runtime),
    };
    const size_t map_size = sizeof(map) / sizeof(yaml_parser_map_t);
    conf_kernel_t *out_k = &out->kernel;
    return yaml_parse_mapping(document, node, map, map_size, out_k);
}

YAML_PARSER_FN(parse_conf_plugins, configuration_t *out)
{
    return yaml_parse_list(
        document, node,
        (yaml_parser_fn_t)parse_conf_plugins_item,
        (vector_t *)&out->plugins, sizeof(conf_plugin_t));
}

YAML_PARSER_FN(parse_conf_acl_rules, configuration_t *out)
{
    // TODO:
    return 0;
}

YAML_PARSER_FN(parse_conf_kernel_rr_timeslice, conf_kernel_t *out)
{
    return yaml_get_long(document, node, &out->rr_timeslice);
}

YAML_PARSER_FN(parse_conf_kernel_sched_rt_period, conf_kernel_t *out)
{
    return yaml_get_long(document, node, &out->sched_rt_period);
}

YAML_PARSER_FN(parse_conf_kernel_sched_rt_runtime, conf_kernel_t *out)
{
    return yaml_get_long(document, node, &out->sched_rt_runtime);
}

YAML_PARSER_FN(parse_conf_plugins_item, conf_plugin_t *out)
{
    const yaml_parser_map_t map[] = {
        YAML_PARSER_MAP_PAIR(key_name, parse_conf_plugins_item_name),
        YAML_PARSER_MAP_PAIR(key_plugin, parse_conf_plugins_item_plugin_path),
        YAML_PARSER_MAP_PAIR(key_priority, parse_conf_plugins_item_priority),
        YAML_PARSER_MAP_PAIR(key_cores, parse_conf_plugins_item_cores),
    };
    const size_t map_size = sizeof(map) / sizeof(yaml_parser_map_t);
    return yaml_parse_mapping(document, node, map, map_size, out);
}

YAML_PARSER_FN(parse_conf_plugins_item_name, conf_plugin_t *out)
{
    return yaml_get_string(document, node, &out->name);
}

YAML_PARSER_FN(parse_conf_plugins_item_plugin_path, conf_plugin_t *out)
{
    return yaml_get_string(document, node, &out->plugin_path);
}

YAML_PARSER_FN(parse_conf_plugins_item_priority, conf_plugin_t *out)
{
    int ret = 0;

    switch (node->type)
    {
    case YAML_SCALAR_NODE:
    {
        // Either a single value is given or a range in the
        // form min-max is provided
        const char delim[] = "-";
        char *strvalue = NULL;
        char *saveptr = NULL;
        char *token = NULL;
        long min = -1, max = -1;

        ret = yaml_get_string(document, node, &strvalue);
        if (ret)
            goto end_scalar;

        token = strtok_r(strvalue, delim, &saveptr);
        if (token == NULL)
        {
            // First token cannot be NULL!
            ret = 1;
            goto end_scalar;
        }

        ret = string_to_long(token, &min);
        if (ret)
        {
            goto end_scalar;
        }

        token = strtok_r(NULL, delim, &saveptr);
        if (token == NULL)
        {
            max = min;
        }
        else
        {
            ret = string_to_long(token, &max);
            if (ret)
            {
                goto end_scalar;
            }
        }

        if (min < 0 || max < 0)
        {
            ret = 1;
            goto end_scalar;
        }

        out->priority_min = min;
        out->priority_max = max;

    end_scalar:
        free(strvalue);
        break;
    }
    case YAML_SEQUENCE_NODE:
    {
        // Either a single value is given or a range in the
        // form [min, max] is provided
        long min = -1, max = -1;
        int i;

        YAML_FOREACH_ITEM(document, node, i)
        {
            switch (i)
            {
            case 0:
                ret = yaml_get_long(
                    document, YAML_NODE_ITEM(document, node, i), &min);
                if (ret)
                    goto end_sequence;
                break;
            case 1:
                ret = yaml_get_long(
                    document, YAML_NODE_ITEM(document, node, i), &max);
                if (ret)
                    goto end_sequence;
                break;
            default:
                ret = 1;
                goto end_sequence;
            }
        }

        if (i == 0)
        {
            // Empty sequence not allowed
            ret = 1;
            goto end_sequence;
        }

        if (i == 1)
        {
            max = min;
        }

        if (min < 0 || max < 0)
        {
            ret = 1;
            goto end_sequence;
        }

        out->priority_min = min;
        out->priority_max = max;

    end_sequence:
        break;
    }
    case YAML_MAPPING_NODE:
    {
        ret = 1;
        break;
    }
    }

    return ret;
}

YAML_PARSER_FN(parse_conf_plugins_item_cores, conf_plugin_t *out)
{
    int ret = 0;

    // This should be a list of all the cores
    switch (node->type)
    {
    case YAML_SCALAR_NODE:
    {
        const char delim[] = ",-";
        char *strvalue = NULL;
        char *saveptr = NULL;
        char *copy = NULL;
        char *token = NULL;
        bool in_range = false;
        long value = 0;
        long num = 0;

        vector_initialize((vector_t *)&out->cores);

        ret = yaml_get_string(document, node, &strvalue);
        if (ret)
            goto end_scalar;

        copy = strdup(strvalue);
        token = strtok_r(strvalue, delim, &saveptr);

        // NOTE: this implementation is broken if multiple delimiters are used
        // one after the other without any number in between
        while (token)
        {
            ret = string_to_long(token, &value);
            if (ret)
                goto end_scalar;

            if (out->cores.size && value <= out->cores.at[out->cores.size - 1])
            {
                ret = 1;
                goto end_scalar;
            }

            if (in_range)
            {
                // There must be at least one prior element
                while (out->cores.at[out->cores.size - 1] < value)
                {
                    vector_realloc(
                        (vector_t *)&out->cores,
                        out->cores.size + 1,
                        sizeof(*out->cores.at));

                    out->cores.at[out->cores.size - 1] =
                        out->cores.at[out->cores.size - 2] + 1;
                }
            }
            else
            {
                vector_realloc(
                    (vector_t *)&out->cores,
                    out->cores.size + 1,
                    sizeof(*out->cores.at));
                out->cores.at[out->cores.size - 1] = value;
            }

            char d = copy[token - strvalue + strlen(token)];

            switch (d)
            {
            case ',':
                in_range = false;
                break;
            case '-':
                if (in_range)
                {
                    ret = 1;
                    goto end_scalar;
                }
                in_range = true;
                break;
            case '\0':
                // Don't care, the next loop will exit
                break;
            default:
                break;
            }

            token = strtok_r(NULL, delim, &saveptr);
        }

    end_scalar:
        free(strvalue);
        free(copy);
        break;
    }
    case YAML_SEQUENCE_NODE:
    {
        ret = 1;

        long value;
        int i;

        YAML_FOREACH_ITEM(document, node, i)
        {
            vector_realloc(
                (vector_t *)&out->cores,
                out->cores.size + 1,
                sizeof(*out->cores.at));

            ret = yaml_get_long(
                document, YAML_NODE_ITEM(document, node, i), &value);
            if (ret)
                goto end_sequence;

            out->cores.at[out->cores.size - 1] = value;
        }

    end_sequence:
        break;
    }
    case YAML_MAPPING_NODE:
    {
        ret = 1;
        break;
    }
    }
    return 0;
}

// ---------------------------------------------------------

int parse_document(const char path[])
{
    bool done;
    int errcode = 0;

    yaml_parser_t parser;
    yaml_document_t document;

    yaml_parser_initialize(&parser);
    FILE *input = fopen(path, "rb");
    yaml_parser_set_input_file(&parser, input);

    configuration_t configuration;
    yaml_node_t *node;

    int res = yaml_parser_load(&parser, &document);
    if (res != 1)
    {
        errcode = 1;
        goto end;
    }
    node = yaml_document_get_root_node(&document);
    // print_node(&document, node);
    parse_conf(&document, node, &configuration);
end:
    yaml_document_delete(&document);
    yaml_parser_delete(&parser);

    return errcode;
}

int main(int argc, char *argv[])
{
    return parse_document(argv[1]);
}
