/**
 * @file list_ptr.h
 * @author Gabriele Serra
 * @date 11 Oct 2018
 * @brief Contains the interface of a linked list of void* 
 *
 * This file contains the interface of a simple implementation of a
 * linked list of void*. This is useful to store in the list any custom
 * element, using the a cast to (void*). This implementation utilizes only 
 * function part of standard library. In case of critical error, the system log
 * mechanism is used to warn the user.
 */

#ifndef LIST_PTR_H
#define LIST_PTR_H

// ---------------------------------------------
// DATA STRUCTURES
// ---------------------------------------------

/**
 * @brief Represent each node of the list
 * 
 * The structure node contains the pointer of the next node
 * in the list and the integer element. If next is NULL
 * that node is the last one.
 */
struct node_ptr {
    struct node_ptr*    next;   /** contains the pointer to next node in list */
    void*               elem;   /** contains the void pointer to the element */
};

/**
 * @brief Represent the list object
 * 
 * The structure list contains the pointer of root node
 * in the list and an integer element that indicate the
 * number of element present in the list. If root is NULL
 * the list is empty.
 */
struct list_ptr {
    int                 n;      /** contains the number of node in the list */
    struct node_ptr*    root;   /** contains the pointer to root node of the list */
};

/**
 * @brief Represent an iterator object
 * 
 * The iterator_t type represent an iterator, used to seek in easy
 * way the list.
 */

typedef struct node_ptr* iterator_t;

// ---------------------------------------------
// MAIN METHODS
// ---------------------------------------------

/**
 * @brief Initialize the list in order to be used
 * 
 * The function ensures that list is in a consistent
 * state before to be used. Put at 0 the number of element
 * and a pointer to NULL in the root.
 * 
 * @param l pointer to the list to be initialized
 */
void list_ptr_init(struct list_ptr* l);

/**
 * @brief Check if the list is empty
 * 
 * Check if the list is empty. Return 0 if the list
 * contains at least one node, or 1 if the list is empty.
 * 
 * @param l pointer to list to be used to be used
 * @return 1 if empty, 0 otherwise
 */
int list_ptr_is_empty(struct list_ptr* l);

/**
 * @brief Return the size of the list
 * 
 * Return the size of the list. If the list
 * is empty, the function will return 0.
 * 
 * @param l pointer to list to be used to be used
 * @return the size of the list or 0 if empty
 */
int list_ptr_get_size(struct list_ptr* l);

/**
 * @brief Add the provided element to the top of the list
 * 
 * Add the provided element to the top of the list
 * 
 * @param l pointer to list to be used
 * @param elem void pointer to element to be added to the list
 */
void list_ptr_add_top(struct list_ptr* l, void* elem);

/**
 * @brief Add the element to the list in a sorted way
 * 
 * The function allocate memory and add the element in the list 
 * in a sorted-way. If the element it's equal to another one,
 * will be put after. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal.
 * 
 * @param l pointer to list to be used
 * @param elem void pointer to element to be added to the list
 * @param cmpfun pointer to the function that will be used to compare elements
 */
void list_ptr_add_sorted(struct list_ptr* l, void* elem, int (* cmpfun)(void* elem1, void* elem2));

/**
 * @brief Remove the top element of the list
 * 
 * Remove the first node of the list and frees memory.
 * If the list is empty, this function does nothing.
 * 
 * @param l: pointer to list to be used
 */
void* list_ptr_remove_top(struct list_ptr* l);

/**
 * @brief Return a pointer to the element contained in the first node
 * 
 * Simply returns a pointer to the element contained in the first node.
 * If the list is empty, the function will return NULL.
 * 
 * @param l pointer to list to be used
 * @return pointer to the element contained in the first node of the list
 */
void* list_ptr_get_top_elem(struct list_ptr* l);

/**
 * @brief Return the pointer to the element contained in the i-th node
 * 
 * Return the i-th element of the list. The list is 0-based. If "i" is
 * greater or equal to the list size, the function returns NULL.
 * 
 * @param l pointer to list to be used
 * @param i the index of the elem to be retrivied
 * @return pointer to the element contained in the i-th node of the list
 */
void* list_ptr_get_i_elem(struct list_ptr* l, unsigned int i);

/**
 * @brief Return the pointer to the i-th node
 * 
 * Return the pointer to i-th node of the list. The list is 0-based. 
 * If "i" is greater or equal to the list size, the function returns NULL.
 * 
 * @param l pointer to list to be used
 * @param i the index of the node to be retrivied
 * @return pointer to the i-th node of the list
 */
struct node_ptr* list_ptr_get_i_node(struct list_ptr* l, unsigned int i);

/**
 * @brief Return the pointer to the node adjacent in the list
 * 
 * Return the pointer to the node adjacent in the list. The caller
 * must ensure that node passed to the function is inside the list.
 * 
 * @param l pointer to list to be used
 * @param node the index of the node that will be used to get the adjacent
 * @return pointer to the adjacent node of the one passed
 */
struct node_ptr* list_ptr_get_next_node(struct list_ptr* l, struct node_ptr* node);

/**
 * @brief Search for the first element and return a pointer to it
 * 
 * Search the list and return a pointer to the first element
 * equal to "elem". If no equal element are found, the function returns
 * NULL. The cmpfun is must be a function that return 0 if elements are equal.
 * 
 * @param l pointer to list to be used
 * @param elem a pointer to the elem that will be searched for
 * @param cmpfun pointer to the function that will be used to compare elements
 * @return pointer to the first element equal to "elem" or NULL if not found
 */
void* list_ptr_search_elem(struct list_ptr* l, void* elem, int (* cmpfun)(void* elem1, void* elem2));

/**
 * @brief Sort (in place) the list
 * 
 * Utilizes an in-place merge sort technique
 * to sort the entire list. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal.
 * 
 * @param l pointer to list to be used
 * @param cmpfun pointer to the function that will be used to compare elements
 */
void list_ptr_sort(struct list_ptr* l, int (* cmpfun)(void* elem1, void* elem2));

/**
 * @brief Removes and returns the searched element from the list
 * 
 * Removes an element from the list @p l if the given @p key is present. In order
 * to compare keys, accept a pointer to a function @p cmpfun. Removes only the
 * first occurence if multiple occurences are present. Returns NULL if no elem
 * was found or the pointer to the element if it was removed with success. 
 * 
 * @param l pointer to the list
 * @param key pointer to the element that must be used as key
 * @param cmpfun pointer to a function used to compare keys
 * @return NULL if no elem was found or a pointer to the element if removed
 */
void* list_ptr_remove(struct list_ptr* l, void* key, int (* cmpfun)(void* elem, void* key));

/**
 * @brief Initializes an iterator to point at the list root
 * 
 * Initializes an iterator make pointing to the first element of the list and
 * returns it
 * 
 * @param l the list that must be seeked
 */
iterator_t iterator_init(struct list_ptr* l);

/**
 * @brief Return the next element of the iterator
 * 
 * Returns the subsequent element of the iterator
 * 
 * @param iterator the iterator to advance
 * @return an iterator advanced by one position
 */
iterator_t iterator_get_next(iterator_t iterator);

/**
 * @brief Returns the element associated with the iterator
 * 
 * Returns the element associated with the current position of an iterator
 * 
 * @param iterator the iterator that points to the position of interest
 * @return the pointer to the element at the position pointer by iterator
 */
void* iterator_get_elem(iterator_t iterator);

#endif

