/**
 * @file retif_config.h
 * @author Gabriele Serra
 * @date 03 Jan 2020
 * @brief Allows to configure kernel parameters for real-time scheduling
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "retif_config.h"
#include "logger.h"
#include "retif_utils.h"
#include "retif_yaml.h"

#define PROC_RT_RUNTIME_FILE "/proc/sys/kernel/sched_rt_runtime_us"
#define PROC_RR_TIMESLICE_FILE "/proc/sys/kernel/sched_rr_timeslice_ms"

int file_read_long(const char *fpath, long *value)
{
    FILE *f = fopen(fpath, "r");
    if (f == NULL)
    {
        LOG(WARNING, "Error opening %s in reading mode.\n", fpath);
        LOG(WARNING, "%s", strerror(errno));
        return -1;
    }

    int res = fscanf(f, "%ld", value);
    fclose(f);

    if (res > 0)
    {
        LOG(DEBUG, "Read %ld from %s.\n", *value, fpath);
        return 0;
    }

    LOG(WARNING, "Could not read from %s.\n", fpath);
    return 1;
}

int file_write(const char *fpath, long value)
{
    FILE *f = fopen(fpath, "w");
    if (f == NULL)
    {
        LOG(WARNING, "Error opening %s in writing mode.\n", fpath);
        LOG(WARNING, "%s", strerror(errno));
        return -1;
    }

    int res = fprintf(f, "%ld", value);
    fclose(f);

    if (res > 0)
    {
        LOG(DEBUG, "Written %ld in %s.\n", value, fpath);
        return 0;
    }

    LOG(WARNING, "Could not write %ld in %s.\n", value, fpath);
    return 1;
}

int set_rt_kernel_params(conf_system_t *c)
{
    int res = 0;
    if ((res = file_write(PROC_RR_TIMESLICE_FILE, c->rr_timeslice)) != 0)
        return res;
    if ((res = file_write(PROC_RT_RUNTIME_FILE, -1)) != 0)
        return res;
    return 0;
}

int save_rt_kernel_params(struct proc_backup *b)
{
    int res = 0;
    if ((res = file_read_long(PROC_RR_TIMESLICE_FILE, &b->rr_timeslice)) != 0)
        return res;
    if ((res = file_read_long(PROC_RT_RUNTIME_FILE, &b->rt_runtime)) != 0)
        return res;
    return 0;
}

int restore_rt_kernel_params(struct proc_backup *b)
{
    int res = 0;
    if ((res = file_write(PROC_RR_TIMESLICE_FILE, b->rr_timeslice)) != 0)
        return res;
    if ((res = file_write(PROC_RT_RUNTIME_FILE, b->rt_runtime)) != 0)
        return res;
    return 0;
}

// ====================================================== //
// --------------- Function Declarations ---------------- //
// ====================================================== //

YAML_PARSER_FN(parse_conf, configuration_t *out);
YAML_PARSER_FN(parse_conf_system, configuration_t *out);
YAML_PARSER_FN(parse_conf_plugins, configuration_t *out);
YAML_PARSER_FN(parse_conf_acl_rules, configuration_t *out);

YAML_PARSER_FN(parse_conf_system_rr_timeslice, conf_system_t *out);
YAML_PARSER_FN(parse_conf_system_sched_max_util, conf_system_t *out);

YAML_PARSER_FN(parse_conf_plugins_item, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_name, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_plugin_path, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_priority, conf_plugin_t *out);
YAML_PARSER_FN(parse_conf_plugins_item_cores, conf_plugin_t *out);

// ====================================================== //
// ---------------- Function Definitions ---------------- //
// ====================================================== //

const char key_conf[] = "configuration";
const char key_system[] = "system";
const char key_plugins[] = "plugins";
const char key_acl_rules[] = "rules";

const char key_rr_timeslice[] = "rr_timeslice";
const char key_sched_max_util[] = "sched_max_util";

const char key_name[] = "name";
const char key_plugin[] = "plugin";
const char key_priority[] = "priority";
const char key_cores[] = "cores";

const char key_domain[] = "domain";
const char key_type[] = "type";
const char key_properties[] = "properties";

// TODO: ACL properties

#define YAML_PARSER_MAP_PAIR(key, ptr)                                         \
    {                                                                          \
        key, (yaml_parser_fn_t) ptr                                            \
    }

YAML_PARSER_FN(parse_conf, configuration_t *out)
{
    const yaml_parser_map_t map[] = {
        YAML_PARSER_MAP_PAIR(key_conf, parse_conf),
        YAML_PARSER_MAP_PAIR(key_system, parse_conf_system),
        YAML_PARSER_MAP_PAIR(key_plugins, parse_conf_plugins),
        YAML_PARSER_MAP_PAIR(key_acl_rules, parse_conf_acl_rules),
    };
    const size_t map_size = sizeof(map) / sizeof(yaml_parser_map_t);
    return yaml_parse_mapping(document, node, map, map_size, out);
}

YAML_PARSER_FN(parse_conf_system, configuration_t *out)
{
    const yaml_parser_map_t map[] = {
        YAML_PARSER_MAP_PAIR(key_rr_timeslice, parse_conf_system_rr_timeslice),
        YAML_PARSER_MAP_PAIR(key_sched_max_util,
            parse_conf_system_sched_max_util),
    };
    const size_t map_size = sizeof(map) / sizeof(yaml_parser_map_t);
    conf_system_t *out_k = &out->system;
    return yaml_parse_mapping(document, node, map, map_size, out_k);
}

YAML_PARSER_FN(parse_conf_plugins, configuration_t *out)
{
    vector_initialize((vector_t *) &out->plugins, VECTOR_ISIZE(out->plugins));

    return yaml_parse_list(document, node,
        (yaml_parser_fn_t) parse_conf_plugins_item, (vector_t *) &out->plugins,
        sizeof(conf_plugin_t));
}

YAML_PARSER_FN(parse_conf_acl_rules, configuration_t *out)
{
    // TODO: ACL properties
    return 0;
}

YAML_PARSER_FN(parse_conf_system_rr_timeslice, conf_system_t *out)
{
    return yaml_get_long(document, node, &out->rr_timeslice);
}

YAML_PARSER_FN(parse_conf_system_sched_max_util, conf_system_t *out)
{
    return yaml_get_double(document, node, &out->sched_max_util);
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

        YAML_FOREACH_ITEM (document, node, i)
        {
            switch (i)
            {
            case 0:
                ret = yaml_get_long(document, YAML_NODE_ITEM(document, node, i),
                    &min);
                if (ret)
                    goto end_sequence;
                break;
            case 1:
                ret = yaml_get_long(document, YAML_NODE_ITEM(document, node, i),
                    &max);
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
    default:
        ret = 1;
        break;
    }

    return ret;
}

YAML_PARSER_FN(parse_conf_plugins_item_cores, conf_plugin_t *out)
{
    int ret = 0;

    vector_initialize((vector_t *) &out->cores, VECTOR_ISIZE(out->cores));

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

            if (out->cores.size &&
                value <= out->cores.data[out->cores.size - 1])
            {
                ret = 1;
                goto end_scalar;
            }

            if (in_range)
            {
                // There must be at least one prior element
                while (out->cores.data[out->cores.size - 1] < value)
                {
                    int newval =
                        (*(int *) vector_back((vector_t *) &out->cores)) + 1;
                    vector_push_back((vector_t *) &out->cores, &newval);
                }
            }
            else
            {
                vector_push_back((vector_t *) &out->cores, &value);
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

        YAML_FOREACH_ITEM (document, node, i)
        {
            ret = yaml_get_long(document, YAML_NODE_ITEM(document, node, i),
                &value);
            if (ret)
                goto end_sequence;

            vector_push_back((vector_t *) &out->cores, &value);
        }

    end_sequence:
        break;
    }
    case YAML_MAPPING_NODE:
    {
        ret = 1;
        break;
    }
    default:
        ret = 1;
        break;
    }
    return 0;
}

// ---------------------------------------------------------

bool configuration_valid(configuration_t *conf);

int parse_configuration(configuration_t *conf, const char path[])
{
    int res = 0;
    yaml_parser_t parser;
    yaml_document_t document;
    yaml_node_t *node;
    FILE *input = NULL;

    input = fopen(path, "rb");
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, input);

    res = yaml_parser_load(&parser, &document);
    if (res != 1)
    {
        LOG(ERR, "Unable to parse %s as a YAML file.\n", path);
        goto end;
    }

    node = yaml_document_get_root_node(&document);
    res = parse_conf(&document, node, conf);
    if (res != 0)
    {
        LOG(ERR, "Problem parsing the configuration file %s.\n", path);
        goto end;
    }

    if (!configuration_valid(conf))
    {
        res = 1;
        goto end;
    }

end:
    yaml_document_delete(&document);
    yaml_parser_delete(&parser);
    fclose(input);

    return res;
}

typedef bool (*conf_check_fn_t)(configuration_t *conf);

bool check_cpus(configuration_t *conf)
{
    conf_plugin_t *p;
    int *c;

    size_t num_procs = get_nprocs2();

    VECTOR_FOREACH (conf->plugins, p)
    {
        VECTOR_FOREACH (p->cores, c)
        {
            if (*c >= num_procs)
            {
                LOG(ERR,
                    "Plugin %s requires CPU %d which is not present in the "
                    "system.\n",
                    p->name, *c);
                return false;
            }
        }
    }

    return true;
}

bool plugins_have_common_cpus(conf_plugin_t *p1, conf_plugin_t *p2)
{
    int i, j;
    for (i = 0, j = 0; i < p1->cores.size && j < p2->cores.size;)
    {
        if (p1->cores.data[i] < p2->cores.data[j])
        {
            ++i;
        }
        else if (p1->cores.data[i] > p2->cores.data[j])
        {
            ++j;
        }
        else
        {
            return true;
        }
    }

    return false;
}

bool check_priority_windows(configuration_t *conf)
{
    size_t high, low;
    conf_plugin_t *phigh, *plow;
    vector_conf_plugin_t *plugins = &conf->plugins;

    for (high = 0; high < plugins->size; ++high)
    {
        for (low = high + 1; low < plugins->size; ++low)
        {
            phigh = &(plugins->data[high]);
            plow = &(plugins->data[low]);
            if (plugins_have_common_cpus(phigh, plow))
            {
                if (phigh->priority_min <= plow->priority_max)
                {
                    LOG(ERR,
                        "Plugin %s shares a core with plugin %s but their "
                        "priorities overlap: [%d-%d] and [%d-%d].\n",
                        phigh->name, plow->name, phigh->priority_min,
                        phigh->priority_max, plow->priority_min,
                        plow->priority_max);
                    return false;
                }
            }
        }
    }

    return true;
}

bool check_minmax_priorities(configuration_t *conf)
{
    conf_plugin_t *p;
    int *c;

    VECTOR_FOREACH (conf->plugins, p)
    {
        if (p->priority_min > p->priority_max)
        {
            LOG(ERR, "Plugin %s has inverted min/max priorities: [%d-%d].\n",
                p->name, p->priority_min, p->priority_max);
            return false;
        }
    }
    return true;
}

bool check_max_util(configuration_t *conf)
{
    if (conf->system.sched_max_util < 0. || conf->system.sched_max_util > 1.)
    {
        LOG(ERR, "Attribute system:sched_max_util out of range [0-1]: %f.\n",
            conf->system.sched_max_util);
        return false;
    }
    return true;
}

bool check_plugin_names(configuration_t *conf)
{
    size_t high, low;
    conf_plugin_t *phigh, *plow;
    vector_conf_plugin_t *plugins = &conf->plugins;

    for (high = 0; high < plugins->size; ++high)
    {
        for (low = high + 1; low < plugins->size; ++low)
        {
            phigh = &(plugins->data[high]);
            plow = &(plugins->data[low]);
            if (strcmp(phigh->name, plow->name) == 0)
            {
                LOG(ERR,
                    "Two plugins (indexes %ld and %ld) share the same name: "
                    "%s.\n",
                    high, low, phigh->name);
                return false;
            }
        }
    }

    return true;
}

bool configuration_valid(configuration_t *conf)
{
    conf_check_fn_t checks[] = {
        check_max_util,
        check_cpus,
        check_minmax_priorities,
        check_priority_windows,
        check_plugin_names,
    };

    for (size_t i = 0; i < sizeof(checks) / sizeof(conf_check_fn_t); ++i)
    {
        if (!checks[i](conf))
        {
            return false;
        }
    }

    return true;
}

// TODO: verbose logging when parsing configuration
