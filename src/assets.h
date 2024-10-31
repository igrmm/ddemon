#ifndef ASSETS_H
#define ASSETS_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "core.h"

enum asset_texture {
    ASSET_TEXTURE_PLAYER = 0,
    ASSET_TEXTURE_TILEMAP,
    ASSET_TEXTURE_ICON_PENCIL,
    ASSET_TEXTURE_ICON_ERASER,
    ASSET_TEXTURE_ICON_ENTITY,
    ASSET_TEXTURE_ICON_SELECT,
    ASSET_TEXTURE_MAX
};

enum asset_font { ASSET_FONT_SMALL = 0, ASSET_FONT_MAX };

enum asset_shader {
    ASSET_SHADER_DEFAULT = 0,
    ASSET_SHADER_ATLAS,
    ASSET_SHADER_PRIMITIVE,
    ASSET_SHADER_MAX
};

struct assets {
    struct asset_atlas *atlas;
    int texture_atlas_indexes[ASSET_TEXTURE_MAX];
    struct txt_font *fonts[ASSET_FONT_MAX];
    Uint32 shaders[ASSET_SHADER_MAX];
};

int assets_load(struct core *core, struct assets *assets);
void assets_dispose(struct assets *assets);
void assets_get_atlas_region(struct asset_atlas *atlas, int index,
                             SDL_FRect *region);
void assets_get_texture_region(struct assets *assets,
                               enum asset_texture asset_texture,
                               SDL_FRect *region);
struct core_texture assets_get_atlas_texture(struct asset_atlas *atlas);

#endif
