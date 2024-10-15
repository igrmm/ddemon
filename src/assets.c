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

#define ASSET_ATLAS_TEXTURE_MAX 1000
#define ASSET_ATLAS_WIDTH 1024
#define ASSET_ATLAS_HEIGHT ASSET_ATLAS_WIDTH
#define ASSET_STB_RECT_PACK_NUM_NODES ASSET_ATLAS_WIDTH * 2

struct asset_atlas {
    stbrp_rect texture_regions[ASSET_ATLAS_TEXTURE_MAX];
    int texture_count;
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

static int assets_atlas_cache_texture(struct asset_atlas *atlas,
                                      struct core_texture texture,
                                      int *texture_region_id)
{
    // check if there is available textures in atlas pool
    if (atlas->texture_count + 1 >= ASSET_ATLAS_TEXTURE_MAX)
        return -1;

    // set out parameter "texture_region_id"
    *texture_region_id = atlas->texture_count;
    atlas->texture_count++;

    // set texture region width and height
    atlas->texture_regions[*texture_region_id].w = texture.width;
    atlas->texture_regions[*texture_region_id].h = texture.height;

    // temporarily use stbrp_reck field "id" to store opengl texture
    atlas->texture_regions[*texture_region_id].id = texture.id;

    return 0;
}

static int assets_atlas_pack_rects(struct asset_atlas *atlas)
{
    struct stbrp_context ctx;
    int num_tex_regions = SDL_arraysize(atlas->texture_regions);
    struct stbrp_node nodes[ASSET_STB_RECT_PACK_NUM_NODES];
    stbrp_init_target(&ctx, ASSET_ATLAS_WIDTH, ASSET_ATLAS_HEIGHT, nodes,
                      ASSET_STB_RECT_PACK_NUM_NODES);
    if (stbrp_pack_rects(&ctx, atlas->texture_regions, num_tex_regions) != 1) {
        SDL_Log("Error packing rectangles for atlas creation.");
        return -1;
    }
    return 0;
}

static void assets_atlas_compute(struct core *core, struct asset_atlas *atlas,
                                 Uint32 atlas_shader)
{
    core_offscreen_rendering_begin(core, &atlas->texture);
    core_update_viewport(core, ASSET_ATLAS_WIDTH, ASSET_ATLAS_HEIGHT);
    core_clear_screen(0.0f, 0.0f, 0.0f, 0.0f);
    core_use_shader(core, atlas_shader);

    // draw textures into atlas
    for (int i = 0; i < atlas->texture_count; i++) {
        float x = atlas->texture_regions[i].x;
        float y = atlas->texture_regions[i].y;
        float w = atlas->texture_regions[i].w;
        float h = atlas->texture_regions[i].h;
        int id = atlas->texture_regions[i].id;
        struct core_texture texture = {w, h, id};
        core_bind_texture(core, texture);
        SDL_FRect src_rect = {0, 0, w, h};
        SDL_FRect dst_rect = {x, y, w, h};
        core_add_drawing_tex(core, NULL, &src_rect, &dst_rect);
        core_draw_queue(core);

        // free tmp texture from gpu's memory
        core_delete_texture(&texture);
        atlas->texture_regions[i].id = 0;
    }
    core_offscreen_rendering_end();
}

static int assets_load_raw(Uint8 *buffer, size_t bufsiz, const char *file_path,
                           size_t *file_size)
{
    SDL_RWops *file = SDL_RWFromFile(file_path, "r");
    if (file == NULL) {
        SDL_Log("Error opening asset: %s (not found?)", file_path);
        return -1;
    }
    for (size_t i = 0; i < bufsiz; i++) {
        if (SDL_RWread(file, &buffer[i], sizeof(Uint8), 1) <= 0) {
            // loading is done, lets wrap it up
            buffer[i] = 0;
            if (file_size != NULL)
                *file_size = i;
            break;
        } else if (i >= bufsiz - 1) {
            // buffer exploded and loading is not done, abort
            SDL_Log("Buffer overflow when loading asset: %s", file_path);
            SDL_RWclose(file);
            return -1;
        }
    }
    SDL_RWclose(file);
    return 0;
}

// to do: split function per asset type
int assets_load(struct core *core, struct assets *assets)
{
    Uint8 buffer[ASSET_BUFSIZ] = {0}; // stack only for now
    const char *file_path = NULL;
    size_t file_size = 0;

    /**
     * Load all shaders
     * */
    for (int i = 0; i < ASSET_SHADER_MAX; i++) {
        // load vertex shader from file using sdl's crossplatform api for files
        Uint8 vert_src[ASSET_BUFSIZ] = {0}; // stack only for now
        file_path = SHADER_VERTEX_PATHS[i];
        if (assets_load_raw(vert_src, ASSET_BUFSIZ, file_path, NULL) != 0)
            return -1;

        // load frag shader from file using sdl's crossplatform api for files
        Uint8 frag_src[ASSET_BUFSIZ] = {0}; // stack only for now
        file_path = SHADER_FRAGMENT_PATHS[i];
        if (assets_load_raw(frag_src, ASSET_BUFSIZ, file_path, NULL) != 0)
            return -1;

        int status;
        char log[512] = {0};
        assets->shaders[i] = core_create_shader(
            (const char *)vert_src, (const char *)frag_src, &status, log, 512);
        if (!status) {
            SDL_Log("shader error: \n\n%s\n", log);
            return -1;
        }
    }

    /**
     * Load all textures
     * */

    // alloc mem for atlas
    assets->atlas = SDL_calloc(1, sizeof(*assets->atlas));
    if (assets->atlas == NULL)
        return -1;
    assets->atlas->texture =
        core_create_stbi_texture(ASSET_ATLAS_WIDTH, ASSET_ATLAS_HEIGHT, 0);

    for (int i = 0; i < ASSET_TEXTURE_MAX; i++) {
        // load img from file using sdl's crossplatform api for files
        buffer[0] = 0;
        file_path = TEXTURE_PATHS[i];
        file_size = 0;
        if (assets_load_raw(buffer, ASSET_BUFSIZ, file_path, &file_size) != 0)
            return -1;

        // load img data from memory using stbi and create the texture
        int width, height, nrChannels;
        Uint8 *texture_data = stbi_load_from_memory(
            buffer, file_size, &width, &height, &nrChannels, STBI_rgb_alpha);
        if (texture_data == NULL) {
            SDL_Log("Failed to loading texture from memory: %s",
                    stbi_failure_reason());
            return -1;
        }

        // store texture in altas for computation
        struct core_texture texture_tmp =
            core_create_stbi_texture(width, height, texture_data);
        int texture_region_id;
        assets_atlas_cache_texture(assets->atlas, texture_tmp,
                                   &texture_region_id);
        assets->texture_region_ids[i] = texture_region_id;
        stbi_image_free(texture_data);
    }

    /**
     * Load all fonts (TO-DO)
     * */

    // cache ASCII codepoints
    struct txt_codepoint_cache *cache = txt_create_codepoint_cache();
    if (cache == NULL)
        return -1;
    for (int i = '!'; i < '~'; i++) {
        char str[] = " ";
        str[0] = i;
        txt_cache_codepoints(cache, str);
    }

    // load font ttf
    buffer[0] = 0;
    file_path = FONT_PATH;
    file_size = 0;
    if (assets_load_raw(buffer, ASSET_BUFSIZ, file_path, &file_size) != 0)
        return -1;

    // create glyph textures
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, buffer, 0)) {
        SDL_Log("Font init failed.");
        return -1;
    }

    assets->fonts[ASSET_FONT_SMALL] = txt_create_font(assets->atlas);
    if (assets->fonts[ASSET_FONT_SMALL] == NULL)
        return -1;

    for (Uint32 codepoint = 0; codepoint < TXT_UNICODE_MAX; codepoint++) {
        if (txt_is_codepoint_cached(cache, codepoint)) {
            int font_size = 64;
            float scale = stbtt_ScaleForPixelHeight(&info, font_size);
            int width, height;
            Uint8 *texture_data = stbtt_GetCodepointBitmap(
                &info, scale, scale, codepoint, &width, &height, 0, 0);
            struct core_texture texture_tmp =
                core_create_stbtt_texture(width, height, texture_data);
            int texture_region_id;
            assets_atlas_cache_texture(assets->atlas, texture_tmp,
                                       &texture_region_id);
            txt_set_glyph(assets->fonts[ASSET_FONT_SMALL], codepoint,
                          texture_region_id);
            SDL_free(texture_data);
        }
    }
    SDL_free(cache);

    /**
     * Make the atlas
     * */
    if (assets_atlas_pack_rects(assets->atlas) < 0)
        return -1;
    assets_atlas_compute(core, assets->atlas,
                         assets->shaders[ASSET_SHADER_ATLAS]);

    return 0;
}

void assets_atlas_get_texture_region(struct asset_atlas *atlas,
                                     int texture_region_id,
                                     SDL_FRect *texture_region)
{
    texture_region->x = atlas->texture_regions[texture_region_id].x;
    texture_region->y = atlas->texture_regions[texture_region_id].y;
    texture_region->w = atlas->texture_regions[texture_region_id].w;
    texture_region->h = atlas->texture_regions[texture_region_id].h;
}

struct core_texture assets_atlas_get_texture(struct asset_atlas *atlas)
{
    return atlas->texture;
}

void assets_dispose(struct assets *assets)
{
    if (assets->atlas != NULL) {
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
