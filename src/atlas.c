#define STB_RECT_PACK_IMPLEMENTATION
#include "../external/stb/stb_rect_pack.h"

#include <SDL3/SDL.h>

#include "atlas.h"

#define ATLAS_REGION_CAPACITY 1000
#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT ATLAS_WIDTH
#define ATLAS_STBRP_NODE_CAPACITY ATLAS_WIDTH * 2

struct atlas {
    stbrp_rect regions[ATLAS_REGION_CAPACITY];
    int region_count;
    struct core_texture texture;
};

struct atlas *atlas_create(void)
{
    struct atlas *atlas = SDL_calloc(1, sizeof(*atlas));
    if (atlas == NULL) {
        SDL_Log("Error allocating memory for atlas (calloc failed)");
        return NULL;
    }
    atlas->texture = core_create_texture(ATLAS_WIDTH, ATLAS_HEIGHT,
                                         CORE_TEXTURE_FORMAT_RGBA, 0);
    return atlas;
}

void atlas_destroy(struct atlas *atlas)
{
    if (atlas != NULL) {
        // delete incomplete atlas's cached opengl texture from gpu
        for (int i = 0; i < ATLAS_REGION_CAPACITY; i++) {
            int id = atlas->regions[i].id;
            if (id > 0) {
                struct core_texture texture = {.id = id};
                core_delete_texture(&texture);
            }
        }
        core_delete_texture(&atlas->texture);
        SDL_free(atlas);
    }
}

bool atlas_cache_texture(struct atlas *atlas, struct core_texture texture,
                         int *index)
{
    // check if there is available regions in atlas
    if (atlas->region_count + 1 >= ATLAS_REGION_CAPACITY) {
        SDL_Log("Error caching texture in atlas: reached max regions.");
        return false;
    }

    // set out parameter "index"
    *index = atlas->region_count;
    atlas->region_count++;

    // set region width and height
    atlas->regions[*index].w = texture.width;
    atlas->regions[*index].h = texture.height;

    // temporarily use stbrp_reck field "id" to store opengl texture
    atlas->regions[*index].id = texture.id;

    return true;
}

bool atlas_pack_rects(struct atlas *atlas)
{
    bool exit_status = true;
    struct stbrp_context ctx;
    struct stbrp_node *nodes =
        SDL_malloc(ATLAS_STBRP_NODE_CAPACITY * sizeof(struct stbrp_node));
    if (nodes == NULL) {
        SDL_Log("Error packing rectangles for atlas creation: malloc failed "
                "(nodes)");
        return false;
    }
    stbrp_init_target(&ctx, ATLAS_WIDTH, ATLAS_HEIGHT, nodes,
                      ATLAS_STBRP_NODE_CAPACITY);
    if (stbrp_pack_rects(&ctx, atlas->regions, atlas->region_count) != 1) {
        SDL_Log("Error packing rectangles for atlas creation.");
        exit_status = false;
    }
    SDL_free(nodes);
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
        float x = atlas->regions[i].x;
        float y = atlas->regions[i].y;
        float w = atlas->regions[i].w;
        float h = atlas->regions[i].h;
        int id = atlas->regions[i].id;
        struct core_texture texture = {w, h, id};
        core_bind_texture(core, texture);
        SDL_FRect src_rect = {0, 0, w, h};
        SDL_FRect dst_rect = {x, y, w, h};
        core_add_drawing_tex(core, NULL, &src_rect, &dst_rect);
        core_draw_queue(core);

        // free tmp texture from gpu's memory
        core_delete_texture(&texture);
        atlas->regions[i].id = 0;
    }
    core_offscreen_rendering_end();
}

void atlas_get_region(struct atlas *atlas, int index, SDL_FRect *region)
{
    region->x = atlas->regions[index].x;
    region->y = atlas->regions[index].y;
    region->w = atlas->regions[index].w;
    region->h = atlas->regions[index].h;
}

struct core_texture atlas_get_texture(struct atlas *atlas)
{
    return atlas->texture;
}
