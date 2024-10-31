/* disable gcc stack-usage warning for this headers */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif

#define STB_RECT_PACK_IMPLEMENTATION
#include "../external/stb/stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../external/stb/stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

/* restore warnings */
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"
#include "txt.h"

#define ASSET_BUFSIZ 512000

#define ASSET_ATLAS_SIZE 1000
#define ASSET_ATLAS_WIDTH 1024
#define ASSET_ATLAS_HEIGHT ASSET_ATLAS_WIDTH
#define ASSET_STBRP_NODES_SIZE ASSET_ATLAS_WIDTH * 2

struct asset_atlas {
    stbrp_rect regions[ASSET_ATLAS_SIZE];
    int count;
    struct core_texture texture;
};

// clang-format off
static const char *TEXTURE_PATHS[] = {
    [ASSET_TEXTURE_PLAYER] = "img.png",
    [ASSET_TEXTURE_TILEMAP] = "opengl.png",
    [ASSET_TEXTURE_ICON_PENCIL] = "pencil.png",
    [ASSET_TEXTURE_ICON_ERASER] = "eraser.png",
    [ASSET_TEXTURE_ICON_ENTITY] = "entity.png",
    [ASSET_TEXTURE_ICON_SELECT] = "select.png",
    [ASSET_TEXTURE_MAX] = 0
};

static const char FONT_PATH[] = "NotoSansMono-Regular.ttf";

static const char *SHADER_VERTEX_PATHS[] = {
    [ASSET_SHADER_DEFAULT] = "default.vs",
    [ASSET_SHADER_ATLAS] = "atlas.vs",
    [ASSET_SHADER_PRIMITIVE] = "primitive.vs",
    [ASSET_SHADER_MAX] = 0
};

static const char *SHADER_FRAGMENT_PATHS[] = {
    [ASSET_SHADER_DEFAULT] = "default.fs",
    [ASSET_SHADER_ATLAS] = "atlas.fs",
    [ASSET_SHADER_PRIMITIVE] = "primitive.fs",
    [ASSET_SHADER_MAX] = 0
};
// clang-format on

static int assets_cache_texture_in_atlas(struct asset_atlas *atlas,
                                         struct core_texture texture,
                                         int *index)
{
    // check if there is available regions in atlas
    if (atlas->count + 1 >= ASSET_ATLAS_SIZE) {
        SDL_Log("Error caching texture in atlas: reached max regions.");
        return -1;
    }

    // set out parameter "index"
    *index = atlas->count;
    atlas->count++;

    // set region width and height
    atlas->regions[*index].w = texture.width;
    atlas->regions[*index].h = texture.height;

    // temporarily use stbrp_reck field "id" to store opengl texture
    atlas->regions[*index].id = texture.id;

    return 0;
}

static int assets_pack_atlas_rects(struct asset_atlas *atlas)
{
    int status = 0;
    struct stbrp_context ctx;
    struct stbrp_node *nodes =
        SDL_malloc(ASSET_STBRP_NODES_SIZE * sizeof(struct stbrp_node));
    if (nodes == NULL) {
        SDL_Log("Error packing rectangles for atlas creation: malloc failed "
                "(nodes)");
        return -1;
    }
    stbrp_init_target(&ctx, ASSET_ATLAS_WIDTH, ASSET_ATLAS_HEIGHT, nodes,
                      ASSET_STBRP_NODES_SIZE);
    if (stbrp_pack_rects(&ctx, atlas->regions, atlas->count) != 1) {
        SDL_Log("Error packing rectangles for atlas creation.");
        status = -1;
    }
    SDL_free(nodes);
    return status;
}

static void assets_compute_atlas(struct core *core, struct asset_atlas *atlas,
                                 Uint32 atlas_shader)
{
    core_offscreen_rendering_begin(core, &atlas->texture);
    core_update_viewport(core, ASSET_ATLAS_WIDTH, ASSET_ATLAS_HEIGHT);
    core_clear_screen(0.0f, 0.0f, 0.0f, 0.0f);
    core_use_shader(core, atlas_shader);

    // draw textures into atlas
    for (int i = 0; i < atlas->count; i++) {
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

static int assets_load_file(Uint8 *buffer, size_t bufsiz, const char *file_path,
                            size_t *file_size)
{
    // try to open file
    SDL_RWops *file = SDL_RWFromFile(file_path, "r");
    if (file == NULL) {
        SDL_Log("%s", SDL_GetError());
        return -1;
    }

    // try to load bytes from file until end of buffer
    int status = 0;
    for (size_t i = 0; i < bufsiz; i++) {
        size_t bytes_read = SDL_RWread(file, &buffer[i], sizeof(Uint8), 1);
        if (bytes_read == 0) {
            // check for error
            const char *error = SDL_GetError();
            if (SDL_strlen(error) != 0) {
                SDL_Log("%s", error);
                status = -1;
                break;
            }
            // loading is done whithout error
            buffer[i] = 0;
            if (file_size != NULL)
                *file_size = i;
            break;
        }

        // check for buffer overflow
        if (i >= bufsiz - 1) {
            SDL_Log("Buffer overflow when loading file: %s", file_path);
            status = -1;
            break;
        }
    }

    if (SDL_RWclose(file) != 0) {
        SDL_Log("%s", SDL_GetError());
        status = -1;
    }

    return status;
}

static int assets_load_shaders(struct assets *assets)
{
    Uint8 *vertex_file_buffer = SDL_malloc(ASSET_BUFSIZ * sizeof(Uint8));
    if (vertex_file_buffer == NULL) {
        SDL_Log("Error loading shaders: malloc failed (vertex_file_buffer)");
        return -1;
    }

    Uint8 *fragment_file_buffer = SDL_malloc(ASSET_BUFSIZ * sizeof(Uint8));
    if (fragment_file_buffer == NULL) {
        SDL_Log("Error loading shaders: malloc failed (fragment_file_buffer)");
        SDL_free(vertex_file_buffer);
        return -1;
    }

    for (int i = 0; i < ASSET_SHADER_MAX; i++) {
        // load vertex shader source from file
        vertex_file_buffer[0] = 0;
        if (assets_load_file(vertex_file_buffer, ASSET_BUFSIZ,
                             SHADER_VERTEX_PATHS[i], NULL) != 0) {
            SDL_free(vertex_file_buffer);
            SDL_free(fragment_file_buffer);
            return -1;
        }

        // load fragment shader source from file
        fragment_file_buffer[0] = 0;
        if (assets_load_file(fragment_file_buffer, ASSET_BUFSIZ,
                             SHADER_FRAGMENT_PATHS[i], NULL) != 0) {
            SDL_free(vertex_file_buffer);
            SDL_free(fragment_file_buffer);
            return -1;
        }

        int status;
        char log[512] = {0};
        const char *vert_src = (const char *)vertex_file_buffer;
        const char *frag_src = (const char *)fragment_file_buffer;
        assets->shaders[i] =
            core_create_shader(vert_src, frag_src, &status, log, 512);
        if (!status) {
            SDL_Log("shader error: \n\n%s\n", log);
            SDL_free(vertex_file_buffer);
            SDL_free(fragment_file_buffer);
            return -1;
        }
    }

    SDL_free(vertex_file_buffer);
    SDL_free(fragment_file_buffer);

    return 0;
}

static int assets_load_textures(struct assets *assets)
{
    size_t file_size = 0;
    Uint8 *file_buffer = SDL_malloc(ASSET_BUFSIZ * sizeof(Uint8));
    if (file_buffer == NULL) {
        SDL_Log("Error loading textures: malloc failed (file_buffer)");
        return -1;
    }

    for (int i = 0; i < ASSET_TEXTURE_MAX; i++) {
        // load img from file
        file_buffer[0] = 0;
        file_size = 0;
        if (assets_load_file(file_buffer, ASSET_BUFSIZ, TEXTURE_PATHS[i],
                             &file_size) != 0) {
            SDL_free(file_buffer);
            return -1;
        }

        // load img data from memory using stbi and create the texture
        int width, height, nrChannels;
        Uint8 *texture_data =
            stbi_load_from_memory(file_buffer, file_size, &width, &height,
                                  &nrChannels, STBI_rgb_alpha);
        if (texture_data == NULL) {
            SDL_Log("Failed to loading texture from memory: %s",
                    stbi_failure_reason());
            SDL_free(file_buffer);
            return -1;
        }

        // store texture in altas for computation
        struct core_texture texture =
            core_create_stbi_texture(width, height, texture_data);
        int index;
        if (assets_cache_texture_in_atlas(assets->atlas, texture, &index) != 0)
            return -1;
        assets->texture_atlas_indexes[i] = index;
        stbi_image_free(texture_data);
    }

    SDL_free(file_buffer);

    return 0;
}

static int assets_load_fonts(struct core *core, struct assets *assets)
{
    // create file buffer
    size_t file_size = 0;
    Uint8 *file_buffer = SDL_malloc(ASSET_BUFSIZ * sizeof(Uint8));
    if (file_buffer == NULL) {
        SDL_Log("Error loading fonts: malloc failed (file_buffer)");
        return -1;
    }

    // cache ascii codepoints
    struct txt_codepoint_cache *cache = txt_create_codepoint_cache();
    if (cache == NULL) {
        SDL_free(file_buffer);
        return -1;
    } else {
        for (int i = '!'; i < '~'; i++) {
            char str[] = " ";
            str[0] = i;
            if (txt_cache_codepoints(cache, str) != 0) {
                SDL_free(file_buffer);
                SDL_free(cache);
                return -1;
            }
        }
    }

    // load ttf file
    if (assets_load_file(file_buffer, ASSET_BUFSIZ, FONT_PATH, &file_size) !=
        0) {
        SDL_free(file_buffer);
        SDL_free(cache);
        return -1;
    }

    // initialize stb true type font
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, file_buffer, 0)) {
        SDL_Log("stbtt_InitFont failed: %s", FONT_PATH);
        SDL_free(file_buffer);
        SDL_free(cache);
        return -1;
    }
    int font_size = 22;
    float scale = stbtt_ScaleForPixelHeight(&info, font_size);

    // create txt_font
    assets->fonts[ASSET_FONT_SMALL] = txt_create_font(assets->atlas);
    if (assets->fonts[ASSET_FONT_SMALL] == NULL) {
        SDL_free(file_buffer);
        SDL_free(cache);
        return -1;
    }

    // get the lowest descent (like the tail in letter 'g'), descent = y1
    int lowest_descent = 0;
    for (Uint32 codepoint = 0; codepoint < TXT_UNICODE_MAX; codepoint++) {
        if (txt_is_codepoint_cached(cache, codepoint)) {
            int descent;
            stbtt_GetCodepointBitmapBox(&info, codepoint, scale, scale, 0, 0, 0,
                                        &descent);
            if (descent > lowest_descent)
                lowest_descent = descent;
        }
    }

    // loop through cached codepoints, create aligned tex and store in atlas
    for (Uint32 codepoint = 0; codepoint < TXT_UNICODE_MAX; codepoint++) {
        if (txt_is_codepoint_cached(cache, codepoint)) {
            int width, height, xoff, yoff;
            Uint8 *texture_data = stbtt_GetCodepointBitmap(
                &info, scale, scale, codepoint, &width, &height, &xoff, &yoff);
            if (texture_data == NULL) {
                SDL_Log("stb_truetype failed getting codepoint bitmap: '%c'",
                        codepoint);
                SDL_free(file_buffer);
                SDL_free(cache);
                return -1;
            }
            struct core_texture texture_boundingbox =
                core_create_stbtt_texture(width, height, texture_data);
            SDL_free(texture_data);

            // align glyph on Y axis with baseline (origin)
            /**
             * The initial texture_boundingbox is made of the bounding box of
             * the glyph without any padding. We want to store these glyphs
             * aligned with the font's baseline (glyph origin point) on
             * atlas. To do this, we will render these bounding boxes a little
             * bit up into another texture (texture_aligned). The glyphs with
             * the lowest descent, like the letter 'g', will not be rendered up,
             * beacause they will be touching the bottom of the texture (they
             * already are), thus, the lowest descent will be used as a
             * reference. For this reason, the final texture will have a bigger
             * height (new_height). STB_TRUE_TYPE uses y-down, so to render up
             * we will be subtracting y. Also, when a glyph have a yoff
             * (top-left of the bounding box to the origin) bigger than the
             * glyph height, in order to align with baseline, we subtract
             * new_height with descent and make descent = 0.
             */
            int new_height = height + lowest_descent;
            int descent = height + yoff;
            if (descent < 0) {
                new_height -= descent;
                descent = 0;
            }

            // align glyph on X axis with baseline (origin)
            int new_width = width + xoff;

            // render to texture
            struct core_texture texture_aligned =
                core_create_stbi_texture(new_width, new_height, 0);
            core_offscreen_rendering_begin(core, &texture_aligned);
            core_update_viewport(core, new_width, new_height);
            core_clear_screen(0.0f, 0.0f, 0.0f, 0.0f);
            core_use_shader(core, assets->shaders[ASSET_SHADER_DEFAULT]);
            core_bind_texture(core, texture_boundingbox);
            SDL_FRect src_rect = {0, 0, width, height};
            SDL_FRect dst_rect = {xoff, descent, width, height};
            core_add_drawing_tex(core, NULL, &src_rect, &dst_rect);
            core_draw_queue(core);
            core_offscreen_rendering_end();

            // store texture into atlas, set glyph on txt_font
            int index;
            if (assets_cache_texture_in_atlas(assets->atlas, texture_aligned,
                                              &index) != 0) {
                SDL_free(file_buffer);
                SDL_free(cache);
                return -1;
            }
            txt_set_glyph_atlas_index(assets->fonts[ASSET_FONT_SMALL],
                                      codepoint, index);

            // unbind texture_boundinbox so we can free it from gpu's memory
            core_bind_texture(core, (struct core_texture){0, 0, 0});
            core_delete_texture(&texture_boundingbox);
        }
    }

    SDL_free(file_buffer);
    SDL_free(cache);

    return 0;
}

int assets_load(struct core *core, struct assets *assets)
{
    assets->atlas = SDL_calloc(1, sizeof(*assets->atlas));
    if (assets->atlas == NULL) {
        SDL_Log("Error allocating memory for atlas in assets_load()");
        return -1;
    }
    assets->atlas->texture =
        core_create_stbi_texture(ASSET_ATLAS_WIDTH, ASSET_ATLAS_HEIGHT, 0);

    if (assets_load_shaders(assets) != 0)
        return -1;

    if (assets_load_textures(assets) != 0)
        return -1;

    if (assets_load_fonts(core, assets) != 0)
        return -1;

    if (assets_pack_atlas_rects(assets->atlas) != 0)
        return -1;

    assets_compute_atlas(core, assets->atlas,
                         assets->shaders[ASSET_SHADER_ATLAS]);

    return 0;
}

void assets_get_atlas_region(struct asset_atlas *atlas, int index,
                             SDL_FRect *region)
{
    region->x = atlas->regions[index].x;
    region->y = atlas->regions[index].y;
    region->w = atlas->regions[index].w;
    region->h = atlas->regions[index].h;
}

void assets_get_texture_region(struct assets *assets,
                               enum asset_texture asset_texture,
                               SDL_FRect *region)
{
    assets_get_atlas_region(
        assets->atlas, assets->texture_atlas_indexes[asset_texture], region);
}

struct core_texture assets_get_atlas_texture(struct asset_atlas *atlas)
{
    return atlas->texture;
}

void assets_dispose(struct assets *assets)
{
    if (assets->atlas != NULL) {
        // delete incomplete atlas's cached opengl texture from gpu
        for (int i = 0; i < ASSET_ATLAS_SIZE; i++) {
            int id = assets->atlas->regions[i].id;
            if (id > 0) {
                struct core_texture texture = {.id = id};
                core_delete_texture(&texture);
            }
        }
        core_delete_texture(&assets->atlas->texture);
        SDL_free(assets->atlas);
    }

    for (int i = 0; i < ASSET_SHADER_MAX; i++) {
        core_delete_shader(assets->shaders[i]);
    }

    for (int i = 0; i < ASSET_FONT_MAX; i++) {
        txt_destroy_font(assets->fonts[i]);
    }
}
