#ifndef ATLAS_H
#define ATLAS_H

#include <SDL3/SDL.h>

#include "core.h"

struct atlas;

struct atlas *atlas_create(void);
void atlas_destroy(struct atlas *atlas);
struct core_texture_region *
atlas_create_region_from_texture(struct atlas *atlas,
                                 struct core_texture texture);
bool atlas_pack_rects(struct atlas *atlas);
void atlas_compute(struct core *core, struct atlas *atlas, Uint32 atlas_shader);
struct core_texture atlas_get_texture(struct atlas *atlas);

#endif
