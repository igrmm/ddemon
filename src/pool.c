#include <stdlib.h>

#include "pool.h"

struct list_node *pool_obtain(struct pool_handle *pool_handle)
{
    if (pool_handle->available.tail == NULL)
        return NULL;
    struct list_node *node = pool_handle->available.tail;
    list_del(node);
    list_add(node, &pool_handle->pooled);
    return node;
}

void pool_return(struct list_node *node, struct pool_handle *pool_handle)
{
    list_del(node);
    list_add(node, &pool_handle->available);
}
