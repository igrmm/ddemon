#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"

#define ASSET_BUFSIZ 1024

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
    [ASSET_SHADER_PRIMITIVE] = "primitive.vs",
    [ASSET_SHADER_MAX] = 0
};

static const char *SHADER_FRAGMENT_PATHS[] = {
    [ASSET_SHADER_DEFAULT] = "default.fs",
    [ASSET_SHADER_PRIMITIVE] = "primitive.fs",
    [ASSET_SHADER_MAX] = 0
};
// clang-format on

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

int assets_load(struct assets *assets)
{
    Uint8 buffer[ASSET_BUFSIZ] = {0}; // stack only for now
    const char *file_path = NULL;
    size_t file_size = 0;

    /**
     * Load all textures
     * */
    stbi_set_flip_vertically_on_load(1);
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
        assets->textures[i] = core_create_texture(width, height, texture_data);
        stbi_image_free(texture_data);
    }

    /**
     * Load all fonts (TO-DO)
     * */
    assets->fonts[ASSET_FONT_SMALL] = NULL;

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

    return 0;
}

void assets_dispose(struct assets *assets)
{
    for (int i = 0; i < ASSET_TEXTURE_MAX; i++) {
        core_delete_texture(&assets->textures[i]);
    }

    for (int i = 0; i < ASSET_SHADER_MAX; i++) {
        core_delete_shader(assets->shaders[i]);
    }
}
