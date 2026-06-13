#include <SDL3/SDL.h>
#include <stddef.h>

#include "arena.h"

#define ALIGNMENT _Alignof(max_align_t)

bool arena_initialize(struct arena *arena, size_t capacity)
{
    arena->start = SDL_malloc(capacity);
    if (arena->start == NULL)
        return false;
    arena->next = arena->start;
    arena->capacity = capacity;
    return true;
}

void arena_terminate(struct arena *arena) { SDL_free(arena->start); }

void *arena_alloc(struct arena *arena, size_t size)
{
    void *ptr = arena->next;

    // align if not aligned
    uintptr_t mod = (uintptr_t)ptr % ALIGNMENT;
    if (mod != 0)
        ptr = (Uint8 *)ptr + ALIGNMENT - mod;

    // check for overflow
    if ((uintptr_t)ptr + size - (uintptr_t)arena->start >= arena->capacity)
        return NULL;

    arena->next = (Uint8 *)ptr + size;

    return ptr;
}

size_t arena_get_used_memory(struct arena *arena)
{
    return ((uintptr_t)arena->next - (uintptr_t)arena->start);
}
