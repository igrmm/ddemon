/** \file
 *
 *  \brief This module implements a simple queue, with enqueue only and no
 *  dequeue.
 *
 *  The queue_handle structure should be used as a "position" handle, as in a
 *  queue, and this position should be treated as a index in a array that
 *  represent the actual queue.
 *
 *  \code
 *  #define CAPACITY 10
 *  struct data;
 *  struct data data_queue[CAPACITY];
 *  struct queue_handle queue_handle;
 *  queue_initialize(&queue_handle, CAPACITY);
 *
 *  int index;
 *  if(queue_add(&index, &queue_handle)) {
 *      //do something with
 *      struct data *my_data = &data_queue[index];
 *  }
 *  \endcode
 *
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <SDL3/SDL.h>

struct queue_handle {
    int capacity;
    int count;
};

/**
 *  Initialize the queue handle with some capacity. Must be the same capacity
 *  as the actual queue array.
 *
 */
void queue_initialize(struct queue_handle *queue_handle, int capacity);

/**
 *  Reset the queue positions.
 *
 */
void queue_reset(struct queue_handle *queue_handle);

/**
 *  Add a position to the queue if it is not full.
 *
 *  \param index must be a valid pointer, and will be given the position value
 *  in the queue.
 *  \param queue_handle a valid pointer to the queue that will be modified.
 *  \returns true if the enqueue succeeds, else returns false.
 *
 */
bool queue_add(int *index, struct queue_handle *queue_handle);

#endif
