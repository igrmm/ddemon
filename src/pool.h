/** \file
 *
 *  \brief This module implements a pool data structure.
 *
 *  The list_node structure should be used as the first member in another data
 *  structure, so it can be casted in order to access the data structure.
 *
 *  \code
 *
 *  struct data_a;
 *  struct data_b;
 *
 *  union my_structure_data {
 *      struct data_a a;
 *      struct data_b b;
 *  };
 *
 *  struct my_structure {
 *      list_node node;
 *      struct my_structure_data data;
 *  };
 *
 *  struct my_structure_poolable {
 *      list_node node;
 *      struct my_structure my_structure;
 *  };
 *
 *  int main(int argc, char *argv[]) {
 *  struct pool_handle pool_handle = {0};
 *  struct my_structure_poolable pool[10];
 *  // init pool to zero
 *  for (int i = 0; i < 10; i++) {
 *      pool[i] = (struct my_structure_poolable){0};
 *      pool_return(&pool[i].node, &pool_handle);
 *  };
 *  return 0;
 *  };
 *
 *  \endcode
 *
 */
#ifndef POOL_H
#define POOL_H

#include "list.h"

struct pool_handle {
    struct list pooled;
    struct list available;
};

struct list_node *pool_obtain(struct pool_handle *pool_handle);
void pool_return(struct list_node *node, struct pool_handle *pool_handle);

#endif
