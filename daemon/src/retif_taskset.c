/**
 * @file retif_taskset.c
 * @author Gabriele Serra
 * @date 11 Oct 2018
 * @brief Contains the implementation of a taskset (linked list of real time task)
 *
 */

#include "retif_taskset.h"

// -----------------------------------------------------
// PRIVATE METHOD
// -----------------------------------------------------

/**
 * @internal
 *
 * The function is simply a wrapper to task compare function
 *
 * @endinternal
 */
static int rtf_taskset_cmp_deadline_asc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, DEADLINE, ASC);
}
static int rtf_taskset_cmp_deadline_dsc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, DEADLINE, DSC);
}
static int rtf_taskset_cmp_priority_asc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, PRIORITY, ASC);
}
static int rtf_taskset_cmp_priority_dsc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, PRIORITY, DSC);
}
static int rtf_taskset_cmp_period_asc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, PERIOD, ASC);
}
static int rtf_taskset_cmp_period_dsc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, PERIOD, DSC);
}
static int rtf_taskset_cmp_wcet_asc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, RUNTIME, ASC);
}
static int rtf_taskset_cmp_wcet_dsc(any_t task1, any_t task2) {
    return task_cmp((struct rtf_task*) task1, (struct rtf_task*) task2, RUNTIME, DSC);
}
static int rtf_taskset_cmp_ppid(void* task, void* ppid) {
    struct rtf_task* t = (struct rtf_task*)task;
    pid_t p = (*(pid_t*)ppid);

    return (t->ptid == p);
}
static int rtf_taskset_cmp_rsvid(void* task, void* rsvid) {
    struct rtf_task* t = (struct rtf_task*)task;
    rtf_id_t p = (*(rtf_id_t*)rsvid);

    return (t->id == p);
}


// -----------------------------------------------------
// PUBLIC METHOD
// -----------------------------------------------------

/**
 * @internal
 *
 * The function ensures that the taskset is in a consistent
 * state before to be used.
 *
 * @endinternal
 */
void rtf_taskset_init(struct rtf_taskset* ts) {
    list_init(&(ts->tasks));
}

/**
 * @internal
 *
 * Check if the taskset is empty. Return 0 if the taskset
 * contains at least one task, or 1 if the taskset is empty.
 *
 * @endinternal
 */
int rtf_taskset_is_empty(struct rtf_taskset* ts) {
    return list_is_empty(&(ts->tasks));
}

/**
 * @internal
 *
 * Return the size of the taskset. If the taskset
 * is empty, the function will return 0.
 *
 * @endinternal
 */
int rtf_taskset_get_size(struct rtf_taskset* ts) {
    return list_get_size(&(ts->tasks));
}

/**
 * @internal
 *
 * Add the provided element to the top of the taskset
 *
 * @endinternal
 */
void rtf_taskset_add_top(struct rtf_taskset* ts, struct rtf_task* task) {
    list_add_top(&(ts->tasks), (void*) task);
}

/**
 * @internal
 *
 * The function allocate memory and add the task in the taskset
 * in a sorted-way. Task with absolute deadline lower will be placed
 * before task with absolute deadline greater. If there is another task
 * already in the taskset with equal deadline, the new task will be
 * put after.
 *
 * @endinternal
 */
void rtf_taskset_add_sorted_dl(struct rtf_taskset* ts, struct rtf_task* task) {
    list_add_sorted(&(ts->tasks), (void*) task, rtf_taskset_cmp_deadline_asc);
}

/**
 * @internal
 *
 * The function allocate memory and add the task in the taskset
 * in a sorted-way. Task with absolute deadline lower will be placed
 * before task with absolute deadline greater. If there is another task
 * already in the taskset with equal deadline, the new task will be
 * put after.
 *
 * @endinternal
 */
struct node_ptr* rtf_taskset_add_sorted_pr(struct rtf_taskset* ts, struct rtf_task* task) {
    return list_add_sorted(&(ts->tasks), (void*) task, rtf_taskset_cmp_period_dsc);
}

/**
 * @internal
 *
 * The function allocate memory and add the task in the taskset
 * in a sorted-way. Task with absolute deadline lower will be placed
 * before task with absolute deadline greater. If there is another task
 * already in the taskset with equal deadline, the new task will be
 * put after.
 *
 * @endinternal
 */
struct node_ptr* rtf_taskset_add_sorted_prio(struct rtf_taskset* ts, struct rtf_task* task) {
    return list_add_sorted(&(ts->tasks), (void*) task, rtf_taskset_cmp_priority_asc);
}

/**
 * @internal
 *
 * Remove the first task of the taskset and frees memory.
 * If the taskset is empty, this function does nothing.
 *
 * @endinternal
 */
struct rtf_task* rtf_taskset_remove_top(struct rtf_taskset* ts) {
    return (struct rtf_task*) list_remove_top(&(ts->tasks));
}

/**
 * @internal
 *
 * Simply returns a pointer to the first task of the list.
 * If the taskset is empty, the function will return NULL.
 *
 * @endinternal
 */
struct rtf_task* rtf_taskset_get_top_task(struct rtf_taskset* ts) {
    return list_get_top_elem(&(ts->tasks));
}

/**
 * @internal
 *
 * Return the pointer to the i-th task in the taskset. The taskset is 0-based.
 * If "i" is greater or equal to the taskset size, the function returns NULL.
 *
 * @endinternal
 */
struct rtf_task* rtf_taskset_get_i_task(struct rtf_taskset* ts, unsigned int i) {
    return list_get_i_elem(&(ts->tasks), i);
}

/**
 * @internal
 *
 * Return the pointer to i-th node of the list. The list is 0-based.
 * If "i" is greater or equal to the list size, the function returns NULL.
 *
 * @endinternal
 */
struct node_ptr* rtf_taskset_get_i_node(struct rtf_taskset* ts, unsigned int i) {
    return list_get_i_node(&(ts->tasks), i);
}

/**
 * @internal
 *
 * Return the pointer to the node adjacent in the taskset. The caller
 * must ensure that node passed to the function is inside the list.
 *
 * @endinternal
 */
struct node_ptr* rtf_taskset_get_next_node(struct rtf_taskset* ts, struct node_ptr* node) {
    return list_get_next_node(&(ts->tasks), node);
}

/**
 * @internal
 *
 * The function ensures that the taskset is in a consistent
 * state before to be used.
 *
 * @endinternal
 */
struct rtf_task* rtf_taskset_search(struct rtf_taskset* ts, rtf_id_t rsvid) {
    return list_search_elem(&(ts->tasks), (void*)&rsvid, rtf_taskset_cmp_rsvid);
}

/**
 * @internal
 *
 * Utilizes an in-place merge sort technique to sort the entire taskset.
 * Use one of WCET, PERIOD, DEADLINE, PRIORITY as enum value PARAM. If
 * the passed parameter is not valid, by default the taskset is sorted by
 * priority. Use ASC as flag to set an ascedent sorting or DSC on the
 * contrary. Other value are not valid, and in this case an ASC sorting
 * will be performed.
 *
 * @endinternal
 */
void rtf_taskset_sort(struct rtf_taskset* ts, enum PARAM p, int flag) {
    switch (p) {
        case PERIOD:
            if(flag == ASC)
                return list_sort(&(ts->tasks), rtf_taskset_cmp_period_asc);
            else
                return list_sort(&(ts->tasks), rtf_taskset_cmp_period_dsc);
        case DEADLINE:
            if(flag == ASC)
                return list_sort(&(ts->tasks), rtf_taskset_cmp_deadline_asc);
            else
                return list_sort(&(ts->tasks), rtf_taskset_cmp_deadline_dsc);
        case PRIORITY:
            if(flag == ASC)
                return list_sort(&(ts->tasks), rtf_taskset_cmp_priority_asc);
            else
                return list_sort(&(ts->tasks), rtf_taskset_cmp_priority_dsc);
        case RUNTIME:
            if(flag == ASC)
                return list_sort(&(ts->tasks), rtf_taskset_cmp_wcet_asc);
            else
                return list_sort(&(ts->tasks), rtf_taskset_cmp_wcet_dsc);
        default:
            break;
    }
}

struct rtf_task* rtf_taskset_remove_by_ppid(struct rtf_taskset* ts, pid_t ppid) {
    return (struct rtf_task*) list_remove(&(ts->tasks), (void*)&ppid, rtf_taskset_cmp_ppid);
}

struct rtf_task* rtf_taskset_remove_by_rsvid(struct rtf_taskset* ts, rtf_id_t rsvid) {
    return (struct rtf_task*) list_remove(&(ts->tasks), (void*)&rsvid, rtf_taskset_cmp_rsvid);
}

void rtf_taskset_remove_all_by_ppid(struct rtf_taskset* ts, pid_t ppid) {
    struct rtf_task* t;

    do {
        t = rtf_taskset_remove_by_ppid(ts, ppid);
    }
    while(t != NULL);

}

iterator_t rtf_taskset_iterator_init(struct rtf_taskset* ts) {
    return iterator_init(&(ts->tasks));
}

iterator_t rtf_taskset_iterator_get_next(iterator_t iterator) {
    return iterator->next;
}

struct rtf_task* rtf_taskset_iterator_get_elem(iterator_t iterator) {
    return (struct rtf_task*) (iterator_get_elem(iterator));
}


/*

#include <stdio.h>
#include <stdlib.h>

void print_list(struct list* l) {
    struct node_ptr* n;
    struct rt_task* t;

    for(n = l->root; n != NULL; n = n->next) {
        t = (struct rt_task*) n->elem;
        printf("%d -> ", get_deadline(t));
    }

    printf("\n");
}

int main() {
    struct rt_taskset ts;
    struct rt_task t1;
    struct rt_task t2;
    struct rt_task t3;


    taskset_init(&ts);

    rt_task_init(&t1, 55, 1);
    set_deadline(&t1, 10);

    rt_task_init(&t2, 56, 1);
    set_deadline(&t2, 5);

    rt_task_init(&t3, 57, 1);
    set_deadline(&t3, 12);

    taskset_add_sorted_dl(&ts, &t3);
    print_list(&(ts.tasks));
    taskset_add_sorted_dl(&ts, &t2);
    print_list(&(ts.tasks));
    taskset_add_sorted_dl(&ts, &t1);

    print_list(&(ts.tasks));

    printf("TEST ID: %d\n", get_deadline(taskset_get_i_task(&ts, 2)));
    //printf("TEST ID: %d\n", get_deadline(taskset_search_elem(&ts, &t1)));

    return 0;
}

*/
