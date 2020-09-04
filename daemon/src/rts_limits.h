
#ifndef RTS_LIMITS_H
#define RTS_LIMITS_H

#include "rts_types.h"
#include "logger.h"
#include "hashmap.h"

// -----------------------------------------------------------------------------
// PLUGINS MACROS / CONSTANTS
// -----------------------------------------------------------------------------

#define RULE_MAX_NAME  10
#define RULE_MAX_NUM   10

#define MAX_CPU 4 // will be removed
#define MAX_USERS 10 // will be removed
#define MAX_GROUPS 10 // will be removed
#define MAX_PLUGINS 4 // will be removed

// -----------------------------------------------------------------------------
// SINGLE LIMIT SYMBOLS
// -----------------------------------------------------------------------------

enum rts_rule_domain
{
    USER,
    GROUP,
    PLUGIN,
    USER_PLUGIN,
    GROUP_PLUGIN
};

enum rts_rule_type
{
    MIN,
    MAX,
    EQUAL
};

struct rts_task;
struct rts_taskset;
struct rts_scheduler;
struct rts_rule;

typedef int (*rts_rule_check_pfun)(struct rts_rule*, struct rts_task*);

typedef int (*rts_rule_update_pfun)(struct rts_rule*, struct rts_scheduler*);

typedef int (*rts_rule_release_pfun)(struct rts_rule*, struct rts_scheduler*);

struct rts_rule
{
    enum rts_rule_type      type;
    char                    name[RULE_MAX_NAME];
    void*                   limit;
    void*                   counter;
    rts_rule_check_pfun     checkfun;
    rts_rule_update_pfun    updatefun;
    rts_rule_release_pfun   releasefun;
};

struct rts_rules
{
    
    int id; // could be uid, gid, plgid
    struct rts_rule* rules[RULE_MAX_NUM];
};

typedef map_t rts_rules_map;

struct rts_limits 
{
    rts_rules_map* u_limits; // <uid, rts_rules>  will be an hash table 1 entry per user 
    rts_rules_map* g_limits; // same above
    rts_rules_map* p_limits; // same above
    rts_rules_map* up_limits; // same above
    rts_rules_map* gp_limits; // same above
};

void rts_limits_allocate_maps(struct rts_limits* l, int unum, int gnum, int pnum);

void rts_limits_rule_register_user(struct rts_limits* l, struct rts_rule* r, uid_t uid);

void rts_limits_rule_register(struct rts_limits* l, struct rts_rule* r, enum rts_rule_domain d, unsigned int id) 
{
    switch (d) 
    {
        case USER:
            rts_limits_rule_register_user(l, r, (uid_t)id);
            break;
        case GROUP:
            rts_limits_rule_register_user(l, r, (gid_t)id); //group
            break;
        case PLUGIN:
            rts_limits_rule_register_user(l, r, (plgid_t)id); // plugin
            break;
        default:
            // can't arrive here!
    }
}

// -----------------------------------------------------------------------------
// RULE TESTING
// -----------------------------------------------------------------------------

int rts_limits_user_rule_exists(uid_t uid, struct rts_limits* l);

// user -> group -> plugin
// user, group acl will be resolved by daemon, plugin acl respective plugin

int rts_limits_check_user_rules(struct rts_scheduler* s, struct rts_limits* l, struct rts_task* t);

#endif /* RTS_LIMITS_H */

// -----------------------------------------------------------------------------
// RULE REGISTERING
// -----------------------------------------------------------------------------

// example

#define CHECK_GE(x, y) (x >= y)
#define CHECK_BE(x, y) (x <= y)
#define CHECK_NE(x, y) (x != y)
#define CHECK_EE(x, y) (x == y)

#define INT_CHECK(x, y, t) CHECK_##t ((int)x, (int)y);

int rts_limits_default_check_int(struct rts_rule* r);

int check_prio_min(struct rts_rule* r, struct rts_task* t);

struct rts_rule util_max;
struct rts_rule runtime_max;
struct rts_rule des_runtime_max;
struct rts_rule deadline_min;
struct rts_rule deadline_max;
struct rts_rule period_min;
struct rts_rule period_max;
struct rts_rule priority_min;
struct rts_rule priority_max;
struct rts_rule ignore_adm;
struct rts_rule enable_plg;