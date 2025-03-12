/** \file
 *
 *  \brief This module implements an intrusive linked list.
 *
 *  The list_node structure should be used as the first member in another data
 *  structure, so it can be casted in order to access the data structure.
 *
 *  \code
 *  struct data {
 *      struct list_node list_node;
 *      int some_data;
 *  };
 *
 *  struct data *data = (struct data *)list_node;
 *  \endcode
 *
 */
#ifndef LIST_H
#define LIST_H

struct list_node {
    struct list_node *next;
    struct list_node *prev;
    struct list *list;
};

struct list {
    struct list_node *head;
    struct list_node *tail;
};

/**
 * Add a node at the end of the list.
 *
 */
void list_add(struct list_node *node, struct list *list);

/**
 * Delete a node from the list that node is part of.
 *
 */
void list_del(struct list_node *node);

/**
 *  Iterate the whole list using the given iterator.
 *
 *  \code
 *  struct list_node *iterator = NULL;
 *  while ((iterator = list_iterate(iterator, &list))) {
 *  //do stuff
 *  }
 *  \endcode
 *
 *  \param iterator must be a null pointer.
 *  \param list a pointer to the list that will be iterated.
 *  \returns iterator with the next node in the list or null at the end of the
 * list.
 *
 * */
struct list_node *list_iterate(struct list_node *iterator, struct list *list);

#endif
