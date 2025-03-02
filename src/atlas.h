#ifndef ATLAS_H
#define ATLAS_H

#include <SDL3/SDL.h>

#include "core.h"

struct atlas;

struct atlas *atlas_create(void);
void atlas_destroy(struct atlas *atlas);
bool atlas_cache_texture(struct atlas *atlas, struct core_texture texture,
                         int *index);
bool atlas_pack_rects(struct atlas *atlas);
void atlas_compute(struct core *core, struct atlas *atlas, Uint32 atlas_shader);
void atlas_get_region(struct atlas *atlas, int index, SDL_FRect *region);
struct core_texture atlas_get_texture(struct atlas *atlas);

#endif
