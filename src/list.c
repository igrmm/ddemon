#include <stdlib.h>

#include "list.h"

void list_add(struct list_node *node, struct list *list)
{
    // node can be added only once
    if (node->list != NULL)
        return;

    node->list = list;

    // the list is empty, add at the beginning
    if (list->head == NULL && list->tail == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }

    // the list is not empty, add at the end
    if (list->tail != node) {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    }
}

void list_del(struct list_node *node)
{
    // node is not in the list
    if (node->list == NULL)
        return;

    // node is the only item in the list
    if (node->list->tail == node && node->list->head == node) {
        node->list->head = NULL;
        node->list->tail = NULL;
        goto reset_node;
    }

    // node is head of the list and list have multiple nodes
    if (node->list->head == node && node->list->tail != node) {
        node->next->prev = NULL;
        node->list->head = node->list->head->next;
        goto reset_node;
    }

    // node is tail of the list and list have multiple nodes
    if (node->list->tail == node && node->list->head != node) {
        node->prev->next = NULL;
        node->list->tail = node->list->tail->prev;
        goto reset_node;
    }

    // node is in the middle of the list
    if (node->prev != NULL && node->next != NULL) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

reset_node:
    node->next = NULL;
    node->prev = NULL;
    node->list = NULL;
}

struct list_node *list_iterate(struct list_node *iterator, struct list *list)
{
    if (list->head == NULL)
        return NULL;

    if (iterator == NULL)
        return list->head;

    if (iterator->next != NULL)
        return iterator->next;

    return NULL;
}
