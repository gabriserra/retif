
#include <stdlib.h>
#include "rts_limits.h"
#include "rts_task.h"

typedef map_t rts_rules_map;

void rts_limits_allocate_map(rts_rules_map* rm, int num) 
{
    int i;

    rm = calloc(num, sizeof(rts_rules_map*));
    
    for (i = 0; i < num; i++)
        rm[i] = hashmap_new();
}

void rts_limits_allocate_maps(struct rts_limits* l, int unum, int gnum, int pnum)
{
    rts_limits_allocate_map(l->u_limits, unum);
    rts_limits_allocate_map(l->g_limits, gnum); 
    rts_limits_allocate_map(l->p_limits, pnum);
    rts_limits_allocate_map(l->up_limits, unum * pnum);
    rts_limits_allocate_map(l->gp_limits, gnum * pnum);
}

void rts_limits_rule_register_user(struct rts_limits* l, struct rts_rule* r, uid_t uid)
{
    // id check

    l->users_limits[uid]->id = uid;
    l->users_limits[uid]->rules[r->id] = r;
}

int rts_limits_default_check_int(struct rts_rule* r)
{
    switch (r->type)
    {
        case MIN:
            return INT_CHECK(r->limit, r->counter, GE);
        case MAX:
            return INT_CHECK(r->limit, r->counter, BE);
        case EQUAL:
            return INT_CHECK(r->limit, r->counter, EE);
        default:
            break;
    }
}

int check_prio_min(struct rts_rule* r, struct rts_task* t) 
{
    r->counter = t->params.priority;
    return rts_limits_default_check_int(r);
}

int rts_limits_user_rule_exists(uid_t uid, struct rts_limits* l)
{
    return (l->users_limits[uid] == NULL); // will be a macro to access data struct
}

int rts_limits_check_user_rules(struct rts_scheduler* s, struct rts_limits* l, struct rts_task* t)
{
    // assuming uid, gid are valids
    uid_t uid = t->euid;
    int result = 1;

    if (rts_limits_user_rule_exists(uid, l) > 0)
    {
        struct rts_rule** rules = l->users_limits[uid]->rules; // TODO: will be an access to map

        for (int i = 0; i < RULE_MAX_NUM; i++)
        {
            struct rts_rule* rule_ith = rules[i];

            if (rule_ith != NULL)
                result = rule_ith->checkfun(rule_ith, t);

            // check failed.
            if (result == 0)
                break;
        }
    }

    return result;
}


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

struct rts_rule prio_min = 
{
    .type       = MIN,
    .name       = "priomin",
    .limit      = NULL,
    .counter    = NULL,
    .checkfun   = check_prio_min,
    .updatefun  = NULL,
    .releasefun = NULL
};