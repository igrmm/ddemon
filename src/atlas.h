#ifndef ATLAS_H
#define ATLAS_H

#include <SDL3/SDL.h>

#include "arena.h"
#include "core.h"

struct stbrp_rect;
typedef struct stbrp_rect stbrp_rect;

struct atlas {
    stbrp_rect *stbrprects;
    struct core_texture_region *regions;
    int region_count;
    struct core_texture texture;
};

bool atlas_initialize(struct atlas *atlas, struct arena *arena);
void atlas_terminate(struct atlas *atlas);
struct core_texture_region *
atlas_create_region_from_texture(struct atlas *atlas,
                                 struct core_texture texture);
bool atlas_pack_rects(struct atlas *atlas, struct arena *arena);
void atlas_compute(struct core *core, struct atlas *atlas, Uint32 atlas_shader);

#endif
