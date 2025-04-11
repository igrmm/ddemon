#include <SDL3/SDL.h>

#include "queue.h"

void queue_initialize(struct queue_handle *queue_handle, int capacity)
{
    *queue_handle = (struct queue_handle){.capacity = capacity};
}

void queue_reset(struct queue_handle *queue_handle) { queue_handle->count = 0; }

static bool queue_is_full(struct queue_handle *queue_handle)
{
    return (queue_handle->count + 1 > queue_handle->capacity);
}

bool queue_add(int *index, struct queue_handle *queue_handle)
{
    if (queue_is_full(queue_handle) || index == NULL)
        return false;
    *index = queue_handle->count++;
    return true;
}
