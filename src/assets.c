/* disable gcc stack-usage warning for this headers */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "../external/stb/stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

/* restore warnings */
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <SDL3/SDL.h>

#include "assets.h"
#include "atlas.h"
#include "core.h"
#include "txt.h"

#define ASSETS_FILE_BUFFER_CAPACITY 512000

// clang-format off
static const char *ASSETS_TEXTURE_PATHS[] = {
    [ASSET_TEXTURE_PLAYER] = "img.png",
    [ASSET_TEXTURE_TILEMAP] = "opengl.png",
    [ASSET_TEXTURE_ICON_PENCIL] = "pencil.png",
    [ASSET_TEXTURE_ICON_ERASER] = "eraser.png",
    [ASSET_TEXTURE_ICON_ENTITY] = "entity.png",
    [ASSET_TEXTURE_ICON_SELECT] = "select.png",
    [ASSET_TEXTURE_PIXEL] = 0,
    [ASSET_TEXTURE_COUNT] = 0
};

static const char ASSETS_FONT_PATH[] = "NotoSansMono-Regular.ttf";

static const char *ASSETS_SHADER_VERTEX_PATHS[] = {
    [ASSET_SHADER_DEFAULT] = "default.vs",
    [ASSET_SHADER_ATLAS] = "atlas.vs",
    [ASSET_SHADER_COUNT] = 0
};

static const char *ASSETS_SHADER_FRAGMENT_PATHS[] = {
    [ASSET_SHADER_DEFAULT] = "default.fs",
    [ASSET_SHADER_ATLAS] = "atlas.fs",
    [ASSET_SHADER_COUNT] = 0
};
// clang-format on

static bool assets_load_file(Uint8 *file_buffer, size_t file_buffer_capacity,
                             const char *file_path, size_t *file_size)
{
    // try to open file
    SDL_IOStream *file = SDL_IOFromFile(file_path, "r");
    if (file == NULL) {
        SDL_Log("%s", SDL_GetError());
        return false;
    }

    // try to load bytes from file until end of buffer
    bool exit_status = true;
    for (size_t i = 0; i < file_buffer_capacity; i++) {
        size_t bytes_read = SDL_ReadIO(file, &file_buffer[i], sizeof(Uint8));
        if (bytes_read == 0) {
            // loading is done whithout error
            if (SDL_GetIOStatus(file) == SDL_IO_STATUS_EOF) {
                file_buffer[i] = 0; // end with nullchar in case of reading text
                if (file_size != NULL)
                    *file_size = i;
                break;
            }
            // check for error
            else {
                SDL_Log("%s", SDL_GetError());
                exit_status = false;
                break;
            }
        }

        // check for buffer overflow
        if (i >= file_buffer_capacity - 1) {
            SDL_Log("Buffer overflow when loading file: %s", file_path);
            exit_status = false;
            break;
        }
    }

    if (!SDL_CloseIO(file)) {
        SDL_Log("%s", SDL_GetError());
        exit_status = false;
    }

    return exit_status;
}

static bool assets_load_shaders(Uint8 *file_buffer, size_t file_buffer_capacity,
                                struct assets *assets)
{
    // split file buffer because we need two buffers
    file_buffer_capacity /= 2;
    Uint8 *vertex_file_buffer = file_buffer;
    Uint8 *fragment_file_buffer = &file_buffer[file_buffer_capacity];

    for (int i = 0; i < ASSET_SHADER_COUNT; i++) {
        // load vertex shader source from file
        vertex_file_buffer[0] = 0;
        if (!assets_load_file(vertex_file_buffer, file_buffer_capacity,
                              ASSETS_SHADER_VERTEX_PATHS[i], NULL))
            return false;

        // load fragment shader source from file
        fragment_file_buffer[0] = 0;
        if (!assets_load_file(fragment_file_buffer, file_buffer_capacity,
                              ASSETS_SHADER_FRAGMENT_PATHS[i], NULL))
            return false;

        int status;
        char log[512] = {0};
        const char *vert_src = (const char *)vertex_file_buffer;
        const char *frag_src = (const char *)fragment_file_buffer;
        assets->shaders[i] =
            core_create_shader(vert_src, frag_src, &status, log, 512);
        if (!status) {
            SDL_Log("shader error: \n\n%s\n", log);
            return false;
        }
    }

    return true;
}

static bool assets_load_textures(Uint8 *file_buffer,
                                 size_t file_buffer_capacity,
                                 struct assets *assets)
{
    size_t file_size = 0;

    for (int i = 0; i < ASSET_TEXTURE_COUNT; i++) {
        // create texture (single white pixel) for primitive drawing
        if (i == ASSET_TEXTURE_PIXEL) {
            Uint8 white_pixel[] = {255, 255, 255, 255};
            struct core_texture texture = core_create_texture(
                1, 1, CORE_TEXTURE_FORMAT_RGBA, white_pixel);
            int index;
            if (!atlas_cache_texture(assets->atlas, texture, &index))
                return false;
            assets->texture_indexes_in_atlas[i] = index;
            continue;
        }

        // load img from file
        file_size = 0;
        if (!assets_load_file(file_buffer, file_buffer_capacity,
                              ASSETS_TEXTURE_PATHS[i], &file_size))
            return false;

        // load img data from memory using stbi and create the texture
        int width, height, nrChannels;
        Uint8 *pixels =
            stbi_load_from_memory(file_buffer, file_size, &width, &height,
                                  &nrChannels, STBI_rgb_alpha);
        if (pixels == NULL) {
            SDL_Log("Failed to loading texture from memory: %s",
                    stbi_failure_reason());
            return false;
        }

        // store texture in altas for computation
        struct core_texture texture = core_create_texture(
            width, height, CORE_TEXTURE_FORMAT_RGBA, pixels);
        int index;
        if (!atlas_cache_texture(assets->atlas, texture, &index)) {
            stbi_image_free(pixels);
            return false;
        }
        assets->texture_indexes_in_atlas[i] = index;
        stbi_image_free(pixels);
    }

    return true;
}

static bool assets_load_fonts(Uint8 *file_buffer, size_t file_buffer_capacity,
                              struct core *core, struct assets *assets)
{
    size_t file_size = 0;

    // cache ascii codepoints
    struct txt_codepoint_cache *cache = txt_create_codepoint_cache();
    if (cache == NULL) {
        return false;
    } else {
        for (int i = ' '; i < '~'; i++) {
            char str[] = " ";
            str[0] = i;
            if (!txt_cache_codepoints(cache, str)) {
                SDL_free(cache);
                return false;
            }
        }
    }

    // load ttf file
    if (!assets_load_file(file_buffer, file_buffer_capacity, ASSETS_FONT_PATH,
                          &file_size)) {
        SDL_free(cache);
        return false;
    }

    // initialize stb true type font
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, file_buffer, 0)) {
        SDL_Log("stbtt_InitFont failed: %s", ASSETS_FONT_PATH);
        SDL_free(cache);
        return false;
    }
    int font_height = 22;
    float scale = stbtt_ScaleForPixelHeight(&info, font_height);

    // create txt_font
    assets->fonts[ASSET_FONT_SMALL] =
        txt_create_font(font_height, assets->atlas);
    if (assets->fonts[ASSET_FONT_SMALL] == NULL) {
        SDL_free(cache);
        return false;
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
    float advance_x = 0.0f;
    for (Uint32 codepoint = 0; codepoint < TXT_UNICODE_MAX; codepoint++) {
        if (txt_is_codepoint_cached(cache, codepoint)) {
            int width, height, xoff, yoff;
            int glyph_index = stbtt_FindGlyphIndex(&info, codepoint);
            if (glyph_index == 0) {
                SDL_Log("font doesn't have glyph for codepoint: %X", codepoint);
                SDL_free(cache);
                return false;
            }
            int advance_x_unscaled = 0;
            stbtt_GetCodepointHMetrics(&info, codepoint, &advance_x_unscaled,
                                       NULL);
            if (advance_x > 0.0f && (advance_x_unscaled * scale) != advance_x) {
                SDL_Log("font is not monospaced, glyph's advance_x vary.");
                SDL_free(cache);
                return false;
            }
            advance_x = advance_x_unscaled * scale;
            float shift_x = advance_x - SDL_floorf(advance_x);
            Uint8 *pixels = stbtt_GetGlyphBitmapSubpixel(
                &info, scale, scale, shift_x, 0.0f, glyph_index, &width,
                &height, &xoff, &yoff);
            if (pixels == NULL && codepoint != ' ') {
                SDL_Log("stb_truetype failed getting codepoint bitmap: '%X'",
                        codepoint);
                SDL_free(cache);
                return false;
            }
            struct core_texture texture_boundingbox = core_create_texture(
                width, height, CORE_TEXTURE_FORMAT_RED, pixels);
            SDL_free(pixels);

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
            struct core_texture texture_aligned = core_create_texture(
                new_width, new_height, CORE_TEXTURE_FORMAT_RGBA, 0);
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
            if (!atlas_cache_texture(assets->atlas, texture_aligned, &index)) {
                SDL_free(cache);
                return false;
            }
            txt_set_glyph_atlas_index(assets->fonts[ASSET_FONT_SMALL],
                                      codepoint, index);

            // unbind texture_boundinbox so we can free it from gpu's memory
            core_bind_texture(core, (struct core_texture){0, 0, 0});
            core_delete_texture(&texture_boundingbox);
        }
    }
    txt_set_font_advance_x(advance_x, assets->fonts[ASSET_FONT_SMALL]);

    SDL_free(cache);

    return true;
}

bool assets_load(struct core *core, struct assets *assets)
{
    assets->atlas = atlas_create();
    if (assets->atlas == NULL)
        return false;

    Uint8 *file_buffer =
        SDL_malloc(ASSETS_FILE_BUFFER_CAPACITY * sizeof(Uint8));
    if (file_buffer == NULL) {
        SDL_Log("Error loading textures: malloc failed (file_buffer)");
        return false;
    }

    bool exit_status = true;

    if (!(exit_status = assets_load_shaders(
              file_buffer, ASSETS_FILE_BUFFER_CAPACITY, assets)))
        goto cleanup;

    if (!(exit_status = assets_load_textures(
              file_buffer, ASSETS_FILE_BUFFER_CAPACITY, assets)))
        goto cleanup;

    if (!(exit_status = assets_load_fonts(
              file_buffer, ASSETS_FILE_BUFFER_CAPACITY, core, assets)))
        goto cleanup;

    if (!(exit_status = atlas_pack_rects(assets->atlas)))
        goto cleanup;

    atlas_compute(core, assets->atlas, assets->shaders[ASSET_SHADER_ATLAS]);

cleanup:
    SDL_free(file_buffer);
    return exit_status;
}

void assets_get_texture_region(struct assets *assets,
                               enum asset_texture asset_texture,
                               SDL_FRect *region)
{
    atlas_get_region(assets->atlas,
                     assets->texture_indexes_in_atlas[asset_texture], region);
}

void assets_dispose(struct assets *assets)
{
    atlas_destroy(assets->atlas);

    for (int i = 0; i < ASSET_SHADER_COUNT; i++) {
        core_delete_shader(assets->shaders[i]);
    }

    for (int i = 0; i < ASSET_FONT_COUNT; i++) {
        txt_destroy_font(assets->fonts[i]);
    }
}
