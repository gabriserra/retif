/**
 * @file list_int.h
 * @author Gabriele Serra
 * @date 11 Oct 2018
 * @brief Contains the interface of a linked list of integers 
 *
 * This file contains the interface of a simple implementation of a
 * linked list of integers. This implementation utilizes only function
 * part of standard library. In case of critical error, the system log
 * mechanism is used to warn the user.
 */

#ifndef LIST_INT_H
#define LIST_INT_H

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
struct node_int {
    struct node_int*    next;   /** contains the pointer to next node in list */
    int                 elem;   /** contains the integer element */
};

/**
 * @brief Represent the list object
 * 
 * The structure list contains the pointer of root node
 * in the list and an integer element that indicate the
 * number of element present in the list. If root is NULL
 * the list is empty.
 */
struct list_int {
    int                 n;      /** contains the number of node in the list */
    struct node_int*    root;   /** contains the pointer to root node of the list */
};

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
void list_int_init(struct list_int* l);

/**
 * @brief Check if the list is empty
 * 
 * Check if the list is empty. Return 0 if the list
 * contains at least one node, or 1 if the list is empty.
 * 
 * @param l pointer to list to be used to be used
 * @return 1 if empty, 0 otherwise
 */
int list_int_is_empty(struct list_int* l);

/**
 * @brief Return the size of the list
 * 
 * Return the size of the list. If the list
 * is empty, the function will return 0.
 * 
 * @param l pointer to list to be used to be used
 * @return the size of the list or 0 if empty
 */
int list_int_get_size(struct list_int* l);

/**
 * @brief Add the provided element to the top of the list
 * 
 * Add the provided element to the top of the list
 * 
 * @param l pointer to list to be used
 * @param elem integer element to be added to the list
 */
void list_int_add_top(struct list_int* l, int elem);

/**
 * @brief Add the element to the list in a sorted way
 * 
 * The function allocate memory and add the element in the list 
 * in a sorted-way. If the element it's equal to another one,
 * will be put after. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal. Use
 * cmp_asc or cmp_dsc if you don't want nothing special.
 * 
 * @param l pointer to list to be used
 * @param elem integer element to be added to the list
 * @param cmpfun pointer to the function that will be used to compare elements
 */
void list_int_add_sorted(struct list_int* l, int elem, int (* cmpfun)(int elem1, int elem2));

/**
 * @brief Remove the top element of the list
 * 
 * Remove the first node of the list and frees memory.
 * If the list is empty, this function does nothing.
 * 
 * @param l: pointer to list to be used
 */
void list_int_remove_top(struct list_int* l);

/**
 * @brief Return a pointer to the element contained in the first node
 * 
 * Simply returns a pointer to the element contained in the first node.
 * If the list is empty, the function will return NULL.
 * 
 * @param l pointer to list to be used
 * @return pointer to the element contained in the first node of the list
 */
int* list_int_get_top_elem(struct list_int* l);

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
int* list_int_get_i_elem(struct list_int* l, unsigned int i);

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
struct node_int* list_int_get_i_node(struct list_int* l, unsigned int i);

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
struct node_int* list_int_get_next_node(struct list_int* l, struct node_int* node);

/**
 * @brief Search for the first element and return a pointer to it
 * 
 * Search the list and return a pointer to the first element
 * equal to "elem". If no equal element are found, the function returns
 * NULL 
 * 
 * @param l pointer to list to be used
 * @param elem the elem that will be searched for
 * @return pointer to the first element equal to "elem" or NULL if not found
 */
int* list_int_search_elem(struct list_int* l, int elem);

/**
 * @brief Remove all elements inside the list and frees memory
 * 
 * Seek the list, remove all nodes and frees memory
 * 
 * @param l pointer to list to be used
 */
void list_int_remove_all(struct list_int* l);

/**
 * @brief Sort (in place) the list
 * 
 * Utilizes an in-place merge sort technique
 * to sort the entire list. The cmpfun pointer function must be a function
 * that return a value greater than 1 is elem1 is greater than elem2,
 * -1 in the opposite case and 0 if elem1 and elem2 are equal. Use
 * cmp_asc or cmp_dsc if you don't want nothing special.
 * 
 * 
 * @param l pointer to list to be used
 * @param cmpfun pointer to the function that will be used to compare elements
 */
void list_int_sort(struct list_int* l, int (* cmpfun)(int elem1, int elem2));

// ---------------------------------------------
// UTILITY
// ---------------------------------------------

/**
 * @brief Compare two integer elements
 * 
 * Return 1 if elem1 is greater than elem2, -1 in the opposite case
 * or 0 if elem1 is equal to elem2. Can be used as compare function
 * in "list_add_sorted" to reach an ASC sorting.
 * 
 * @param elem1 the first integer element
 * @param elem2 the second integer element
 * @return 1 if elem1 > elem2, 0 if elem1 == elem2, -1 if elem1 < elem2
 */
int int_cmp_asc(int elem1, int elem2);

/**
 * @brief Compare two integer elements
 * 
 * Return -1 if elem1 is greater than elem2, 1 in the opposite case
 * or 0 if elem1 is equal to elem2. Can be used as compare function
 * in "list_add_sorted" to reach an DSC sorting.
 * 
 * @param elem1 the first integer element
 * @param elem2 the second integer element
 * @return -1 if elem1 > elem2, 0 if elem1 == elem2, 1 if elem1 < elem2
 */
int int_cmp_dsc(int elem1, int elem2);

#endif
