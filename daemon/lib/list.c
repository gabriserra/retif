/**
 * @file list.c
 * @author Gabriele Serra
 * @date 11 Oct 2018
 * @brief Contains the implementation of a linked list of any_t 
 *
 */

#include "list.h"
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

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
static any_t alloc(int count, size_t dimension) {
	any_t ret = calloc(count, dimension);

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
static void split(struct node_ptr* l, struct node_ptr** l1_ptr, struct node_ptr** l2_ptr) {
    struct node_ptr* fast; 
    struct node_ptr* slow; 
    
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
static struct node_ptr* merge(struct node_ptr* l1, struct node_ptr* l2, int (* cmpfun)(any_t elem1, any_t elem2)) {
    struct node_ptr* res; 
  
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
static void merge_sort(struct node_ptr** l_ptr, int (* cmpfun)(any_t elem1, any_t elem2)) {
    struct node_ptr* l;
    struct node_ptr* l1; 
    struct node_ptr* l2;

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
void list_init(struct list* l) {
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
int list_is_empty(struct list* l) {
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
int list_get_size(struct list* l) {
    return l->n;
}

/**
 * @internal
 *
 * Add the provided element to the top of the list
 * 
 * @endinternal
 */
void list_add_top(struct list* l, any_t elem) {
    struct node_ptr* n = alloc(1, sizeof(struct node_ptr));

    n->next = l->root;
    n->elem = elem;

    l->n++;
    l->root = n;
}

/**
 * @internal
 *
 * The function allocate memory and add the element in the list 
 * in a sorted-way. If the element it's equal to another one,
 * will be put after. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal.
 * 
 * @endinternal
 */
struct node_ptr* list_add_sorted(struct list* l, any_t elem, int (* cmpfun)(any_t elem1, any_t elem2)) {
    struct node_ptr* seek;
    struct node_ptr* prec;
    struct node_ptr* new;

    if(list_is_empty(l) || cmpfun(elem, l->root->elem) < 0) {
        list_add_top(l, elem);
        return l->root;
    }

    prec = l->root;
    new = alloc(1, sizeof(struct node_ptr));
    new->elem = elem;

    for(seek = l->root->next; seek != NULL && cmpfun(elem, seek->elem) > 0;) {
        prec = prec->next;
        seek = seek->next;
    }

    prec->next = new;
    new->next = seek;
    l->n++;

    return new;
}

/**
 * @internal
 *
 * Remove the first node of the list and frees memory.
 * If the list is empty, this function does nothing.
 *  
 * @endinternal
 */
any_t list_remove_top(struct list* l) {
    any_t elem;
    struct node_ptr* n;

    if(list_is_empty(l))
        return NULL;
    
    n = l->root;
    l->root = l->root->next;
    l->n--;
    
    elem = n->elem;
    free(n);
    
    return elem;
}

/**
 * @internal
 *
 * Simply returns a pointer to the element contained in the first node.
 * If the list is empty, the function will return NULL.
 *  
 * @endinternal
 */
any_t list_get_top_elem(struct list* l) {
    if(list_is_empty(l))
        return NULL;

    return l->root->elem;
}

/**
 * @internal
 * 
 * Return the i-th element of the list. The list is 0-based. If "i" is
 * greater or equal to the list size, the function returns NULL.
 * 
 * @endinternal
 */
any_t list_get_i_elem(struct list* l, unsigned int i) {
    struct node_ptr* n;

    n = list_get_i_node(l, i);

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
struct node_ptr* list_get_i_node(struct list* l, unsigned int i) {
    int j;
    struct node_ptr* n;

    if(list_get_size(l) < i + 1)
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
struct node_ptr* list_get_next_node(struct list* l, struct node_ptr* node) {
    if(node == NULL)
        return NULL;
        
    return node->next;
}

/**
 * @internal
 * 
 * Search the list and return a pointer to the first element
 * equal to "elem". If no equal element are found, the function returns
 * NULL. The cmpfun is must be a function that return 0 if elements are equal. 
 * 
 * @endinternal
 */
any_t list_search_elem(struct list* l, any_t elem, int (* cmpfun)(any_t elem1, any_t elem2)) {
    struct node_ptr* n;

    for(n = l->root; n != NULL; n = n->next)
        if(!cmpfun(n->elem, elem))
            return n->elem;

    return NULL;
}

/**
 * @internal
 * 
 * Utilizes an in-place merge sort technique
 * to sort the entire list. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal.
 * 
 * @endinternal
 */
void list_sort(struct list* l, int (* cmpfun)(any_t elem1, any_t elem2)) {
    merge_sort(&(l->root), cmpfun);
}

/**
 * @internal
 * 
 * Removes an element from the list @p l if the given @p key is present. In order
 * to compare keys, accept a pointer to a function @p cmpfun. Removes only the
 * first occurence if multiple occurences are present. Returns NULL if no elem
 * was found or the pointer to the element if it was removed with success. 
 * 
 * @endinternal
 */
any_t list_remove(struct list* l, any_t key, int (* cmpfun)(any_t elem, any_t key)) {
    any_t elem;
    struct node_ptr* seek;
    struct node_ptr* prec;
        
    if(list_is_empty(l))
        return NULL;
    
    if(cmpfun(l->root->elem, key))
        return list_remove_top(l);

    prec = l->root;

    for(seek = l->root->next; seek != NULL && cmpfun(seek->elem, key) != 0;) {
        prec = prec->next;
        seek = seek->next;
    }
    
    if(seek == NULL)
        return NULL;

    prec->next = seek->next;
    elem = seek->elem;
    
    l->n--;
    free(seek);
    
    return elem;
}

/**
 * @internal
 * 
 * Initialized an iterator make pointing to the first element of the list and
 * returns it
 * 
 * @endinternal
 */
iterator_t iterator_init(struct list* l) {
    return l->root;
}

/**
 * @internal
 * 
 * Returns the subsequent element of the iterator
 * 
 * @endinternal
 */
iterator_t iterator_get_next(iterator_t iterator) {
    return iterator->next;
}

/**
 * @internal
 * 
 * Returns the element associated with the current position of an iterator
 * 
 * @endinternal
 */
any_t iterator_get_elem(iterator_t iterator) {
    return iterator->elem;
}
