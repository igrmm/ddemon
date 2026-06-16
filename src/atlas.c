#define STB_RECT_PACK_IMPLEMENTATION
#include "../external/stb/stb_rect_pack.h"

#include <SDL3/SDL.h>

#include "arena.h"
#include "atlas.h"

#define ATLAS_CAPACITY 1000
#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT ATLAS_WIDTH
#define ATLAS_STBRP_NODE_CAPACITY ATLAS_WIDTH * 2

bool atlas_initialize(struct atlas *atlas, struct arena *arena)
{
    atlas->stbrprects = arena_alloc(arena, ATLAS_CAPACITY * sizeof(stbrp_rect));
    if (atlas->stbrprects == NULL) {
        SDL_Log("Error initializing atlas->stbrprects: arena_alloc failed.");
        return false;
    }

    atlas->regions =
        arena_alloc(arena, ATLAS_CAPACITY * sizeof(struct core_texture_region));
    if (atlas->regions == NULL) {
        SDL_Log("Error initializing atlas->regions: arena_alloc failed.");
        return false;
    }

    for (int i = 0; i < ATLAS_CAPACITY; i++) {
        atlas->stbrprects[i] = (stbrp_rect){0};
        atlas->regions[i] = (struct core_texture_region){0};
    }
    atlas->region_count = 0;
    atlas->texture = core_create_texture(ATLAS_WIDTH, ATLAS_HEIGHT,
                                         CORE_TEXTURE_FORMAT_RGBA, 0);
    return atlas;
}

void atlas_terminate(struct atlas *atlas)
{
    core_delete_texture(&atlas->texture);
    // delete incomplete atlas's cached opengl texture from gpu
    if (atlas->stbrprects == NULL)
        return;
    for (int i = 0; i < ATLAS_CAPACITY; i++) {
        int id = atlas->stbrprects[i].id;
        if (id > 0) {
            struct core_texture texture = {.id = id};
            core_delete_texture(&texture);
        }
    }
}

struct core_texture_region *
atlas_create_region_from_texture(struct atlas *atlas,
                                 struct core_texture texture)
{
    // check if there is available regions in atlas
    if (atlas->region_count + 1 >= ATLAS_CAPACITY) {
        SDL_Log("Error caching texture in atlas: reached max regions.");
        return NULL;
    }

    int index = atlas->region_count;
    atlas->region_count++;

    atlas->stbrprects[index].w = texture.width;
    atlas->stbrprects[index].h = texture.height;

    // temporarily use stbrp_reck field "id" to store opengl texture
    atlas->stbrprects[index].id = texture.id;

    return &atlas->regions[index];
}

bool atlas_pack_rects(struct atlas *atlas, struct arena *arena)
{
    bool exit_status = true;
    struct stbrp_context ctx;
    struct stbrp_node *nodes = arena_alloc(
        arena, ATLAS_STBRP_NODE_CAPACITY * sizeof(struct stbrp_node));
    if (nodes == NULL) {
        SDL_Log(
            "Error packing rectangles for atlas creation: arena_alloc failed.");
        return false;
    }
    stbrp_init_target(&ctx, ATLAS_WIDTH, ATLAS_HEIGHT, nodes,
                      ATLAS_STBRP_NODE_CAPACITY);
    if (stbrp_pack_rects(&ctx, atlas->stbrprects, atlas->region_count) != 1) {
        SDL_Log("Error packing rectangles for atlas creation.");
        exit_status = false;
    }
    return exit_status;
}

void atlas_compute(struct core *core, struct atlas *atlas, Uint32 atlas_shader)
{
    core_offscreen_rendering_begin(core, &atlas->texture);
    core_update_viewport(core, ATLAS_WIDTH, ATLAS_HEIGHT);
    core_clear_screen(0.0f, 0.0f, 0.0f, 0.0f);
    core_use_shader(core, atlas_shader);

    // draw textures into atlas
    for (int i = 0; i < atlas->region_count; i++) {
        float x = atlas->regions[i].rect.x = atlas->stbrprects[i].x;
        float y = atlas->regions[i].rect.y = atlas->stbrprects[i].y;
        float w = atlas->regions[i].rect.w = atlas->stbrprects[i].w;
        float h = atlas->regions[i].rect.h = atlas->stbrprects[i].h;
        int id = atlas->stbrprects[i].id;
        struct core_texture texture = {w, h, id};
        core_bind_texture(core, texture);
        SDL_FRect src_rect = {0, 0, w, h};
        SDL_FRect dst_rect = {x, y, w, h};
        core_add_drawing_tex(core, &src_rect, &dst_rect);
        core_render_drawings(core);

        // free tmp texture from gpu's memory
        core_delete_texture(&texture);
        atlas->stbrprects[i].id = 0;
    }
    core_offscreen_rendering_end();
}
