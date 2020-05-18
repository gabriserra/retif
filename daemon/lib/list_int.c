/**
 * @file list_int.c
 * @author Gabriele Serra
 * @date 11 Oct 2018
 * @brief Contains the implementation of a linked list of integers 
 *
 */

#include "list_int.h"
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

// -----------------------------------------------------
// DEBUG METHOD
// -----------------------------------------------------

#ifdef DEBUG

#include <stdio.h>

/**
 * @internal
 *
 * The functions seek the list and prints each element
 * followed by an arrow ->. Remove #define DEBUG to hide
 * this function
 * 
 * @endinternal
 */
void print_list(struct list_int* l) {
    struct node_int* n;
    
    for(n = l->root; n != NULL; n = n->next) {
        printf("%d -> ", n->elem);
    }

    printf("\n");
}

#endif

// -----------------------------------------------------
// PRIVATE METHOD
// -----------------------------------------------------

/**
 * @internal
 *
 * The function allocate count * dimension memory in the heap.
 * If the system can not allocate memory, an error message
 * will be placed in the system log and the programs terminate.
 * 
 * @endinternal
 */
static void* alloc(int count, size_t dimension) {
	void* ret = calloc(count, dimension);

	// check if operation was performed
	if(ret != NULL)
		return ret;

	// print and exit
    syslog(LOG_ALERT, "The system is out of memory: %s", strerror(errno));
	exit(-1);
}

/**
 * @internal
 * 
 * Split the nodes of the given list into front and back halves,
 * and return the two lists using the reference parameters. 
 * If the length is odd, the extra node should go in the front list. 
 * Uses the fast/slow pointer strategy.
 * 
 * @endinternal
 */
static void split(struct node_int* l, struct node_int** l1_ptr, struct node_int** l2_ptr) {
    struct node_int* fast; 
    struct node_int* slow; 
    
    slow = l; 
    fast = l->next; 
  
    while (fast != NULL) { 
        fast = fast->next; 
        
        if (fast != NULL) { 
            slow = slow->next; 
            fast = fast->next; 
        } 
    } 
  
    *l1_ptr = l; 
    *l2_ptr = slow->next;
    slow->next = NULL;     
}

/**
 * @internal
 *
 * The function merge two node l1 and l2 and return the
 * pointer to a new node list that contains the two nodes merged
 * Note that the function is recursive!
 * 
 * @endinternal
 */
static struct node_int* merge(struct node_int* l1, struct node_int* l2, int (* cmpfun)(int elem1, int elem2)) {
    struct node_int* res; 
  
    if (l1 == NULL) 
        return l1; 
    else if (l2 == NULL) 
        return l2; 
  
    if(cmpfun(l1->elem, l2->elem) < 0) {
        res = l1; 
        res->next = merge(l1->next, l2, cmpfun); 
    } else { 
        res = l2; 
        res->next = merge(l1, l2->next, cmpfun); 
    } 
    
    return res; 
}

/**
 * @internal
 *
 * The function recursively split the list in two, sort each
 * of the two halves and merge it.
 * 
 * @endinternal
 */
static void merge_sort(struct node_int** l_ptr, int (* cmpfun)(int elem1, int elem2)) {
    struct node_int* l;
    struct node_int* l1; 
    struct node_int* l2;

    l = *l_ptr;     
    
    if ((l == NULL) || (l->next == NULL)) 
        return;
    
    split(l, &l1, &l2);
    
    merge_sort(&l1, cmpfun); 
    merge_sort(&l2, cmpfun); 
    
    *l_ptr = merge(l1, l2, cmpfun); 
}

// -----------------------------------------------------
// PUBLIC METHOD
// -----------------------------------------------------

/**
 * @internal
 *
 * The function ensures that list is in a consistent
 * state before to be used. Put at 0 the number of element
 * and a pointer to NULL in the root.
 * 
 * @endinternal
 */
void list_int_init(struct list_int* l) {
    l->n = 0;
    l->root = NULL;
}

/**
 * @internal
 *
 * Check if the list is empty. Return 0 if the list
 * contains at least one node, or 1 if the list is empty.
 * 
 * @endinternal
 */
int list_int_is_empty(struct list_int* l) {
    if(!l->n)
        return 1;
    return 0;
}

/**
 * @internal
 *
 * Return the size of the list. If the list
 * is empty, the function will return 0.
 * 
 * @endinternal
 */
int list_int_get_size(struct list_int* l) {
    return l->n;
}

/**
 * @internal
 *
 * Add the provided element to the top of the list
 * 
 * @endinternal
 */
void list_int_add_top(struct list_int* l, int elem) {
    struct node_int* new = alloc(1, sizeof(struct node_int));

    new->next = l->root;
    new->elem = elem;

    l->n++;
    l->root = new;
}

/**
 * @internal
 *
 * The function allocate memory and add the element in the list 
 * in a sorted-way. If the element it's equal to another one,
 * will be put after. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal. Use
 * int_cmp_asc or int_cmp_dsc if you don't want nothing special.
 * 
 * @endinternal
 */
void list_int_add_sorted(struct list_int* l, int elem, int (* cmpfun)(int elem, int lelem)) {
    struct node_int* seek;
    struct node_int* prec;
    struct node_int* new;

    if(list_int_is_empty(l) || cmpfun(elem, l->root->elem) < 0) {
        list_int_add_top(l, elem);
        return;
    }

    prec = l->root;
    new = alloc(1, sizeof(struct node_int));
    new->elem = elem;

    for(seek = l->root->next; seek != NULL && cmpfun(elem, seek->elem) > 0;) {
        prec = prec->next;
        seek = seek->next;
    }
        
    prec->next = new;
    new->next = seek;
    l->n++;
}

/**
 * @internal
 *
 * Remove the first node of the list and frees memory.
 * If the list is empty, this function does nothing.
 *  
 * @endinternal
 */
void list_int_remove_top(struct list_int* l) {
    struct node_int* n;

    if(list_int_is_empty(l))
        return;
    
    n = l->root;
    l->root = l->root->next;
    l->n--;
    
    free(n);
}

/**
 * @internal
 *
 * Simply returns a pointer to the element contained in the first node.
 * If the list is empty, the function will return NULL.
 *  
 * @endinternal
 */
int* list_int_get_top_elem(struct list_int* l) {
    if(list_int_is_empty(l))
        return NULL;
    
    return &(l->root->elem);
}

/**
 * @internal
 * 
 * Return the i-th element of the list. The list is 0-based. If "i" is
 * greater or equal to the list size, the function returns NULL.
 * 
 * @endinternal
 */
int* list_int_get_i_elem(struct list_int* l, unsigned int i) {
    struct node_int* n;

    n = list_int_get_i_node(l, i);

    if(n == NULL)
        return NULL;

    return &(n->elem);
}

/**
 * @internal
 * 
 * Return the pointer to i-th node of the list. The list is 0-based. 
 * If "i" is greater or equal to the list size, the function returns NULL.
 * 
 * @endinternal
 */
struct node_int* list_int_get_i_node(struct list_int* l, unsigned int i) {
    int j;
    struct node_int* n;

    if(list_int_get_size(l) < i + 1)
        return NULL;

    n = l->root;
    
    for(j = 0; j < i; j++)
        n = n->next;

    return n;
}

/**
 * @internal
 * 
 * Return the pointer to the node adjacent in the list. The caller
 * must ensure that node passed to the function is inside the list.
 * 
 * @endinternal
 */
struct node_int* list_int_get_next_node(struct list_int* l, struct node_int* node) {
    if(node == NULL)
        return NULL;
        
    return node->next;
}

/**
 * @internal
 * 
 * Search the list and return a pointer to the first element
 * equal to "elem". If no equal element are found, the function returns
 * NULL 
 * 
 * @endinternal
 */
int* list_int_search_elem(struct list_int* l, int elem) {
    struct node_int* n;

    for(n = l->root; n != NULL; n = n->next)
        if(n->elem == elem)
            return &(n->elem);

    return NULL;
}

/**
 * @internal
 * 
 * Seek the list, remove all nodes and frees memory
 * 
 * @endinternal
 */
void list_int_remove_all(struct list_int* l) {
    struct node_int* n;
    struct node_int* p;

    if(list_int_is_empty(l))
        return;

    for(n = l->root; n != NULL; n = n->next) {
        p = n;
        n = n->next;
        free(p);
    }

    list_int_init(l);
}

/**
 * @internal
 * 
 * Utilizes an in-place merge sort technique
 * to sort the entire list. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal. Use
 * cmp_asc or cmp_dsc if you don't want nothing special.
 * 
 * @endinternal
 */
void list_int_sort(struct list_int* l, int (* cmpfun)(int elem1, int elem2)) {
    merge_sort(&(l->root), cmpfun);
}

// ---------------------------------------------
// UTILITY
// ---------------------------------------------

/**
 * @internal
 *
 * Return 1 if elem1 is greater than elem2, -1 in the opposite case
 * or 0 if elem1 is equal to elem2. Can be used as compare function
 * in "list_add_sorted" to reach an ASC sorting.
 *  
 * @endinternal
 */
int int_cmp_asc(int elem1, int elem2) {
    if(elem1 > elem2)
        return 1;
    else if(elem1 < elem2)
        return -1;
    else
        return 0;
}

/**
 * @internal
 *
 * Return 1 if elem1 is greater than elem2, -1 in the opposite case
 * or 0 if elem1 is equal to elem2. Can be used as compare function
 * in "list_add_sorted" to reach an ASC sorting.
 *  
 * @endinternal
 */
int int_cmp_dsc(int elem1, int elem2) {
    if(elem1 > elem2)
        return -1;
    else if(elem1 < elem2)
        return 1;
    else
        return 0;
}

