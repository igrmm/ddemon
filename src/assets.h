#ifndef ASSETS_H
#define ASSETS_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "core.h"

enum asset_texture {
    ASSET_TEXTURE_PLAYER = 0,
    ASSET_TEXTURE_TILEMAP,
    ASSET_TEXTURE_MAX
};
enum asset_font { ASSET_FONT_SMALL = 0, ASSET_FONT_MAX };
enum asset_shader { ASSET_SHADER_DEFAULT = 0, ASSET_SHADER_MAX };

struct assets {
    struct core_texture textures[ASSET_TEXTURE_MAX];
    struct txt_font *fonts[ASSET_FONT_MAX];
    Uint32 shaders[ASSET_SHADER_MAX];
};

int assets_load(struct assets *assets);
void assets_dispose(struct assets *assets);

#endif
