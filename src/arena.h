/** \file
 *
 *  \brief This module implements a naive memory arena.
 *
 */
#ifndef ARENA_H
#define ARENA_H

#include <SDL3/SDL.h>

struct arena {
    void *start;
    void *next;
    size_t capacity;
};

bool arena_initialize(struct arena *arena, size_t capacity);
void arena_terminate(struct arena *arena);
void *arena_alloc(struct arena *arena, size_t size);
size_t arena_get_used_memory(struct arena *arena);

#endif
