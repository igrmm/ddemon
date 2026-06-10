#ifndef ASSETS_H
#define ASSETS_H

#include <SDL3/SDL.h>

#include "core.h"

struct atlas;

enum asset_texture {
    ASSET_TEXTURE_PLAYER = 0,
    ASSET_TEXTURE_TILEMAP,
    ASSET_TEXTURE_ICON_PENCIL,
    ASSET_TEXTURE_ICON_ERASER,
    ASSET_TEXTURE_ICON_ENTITY,
    ASSET_TEXTURE_ICON_SELECT,
    ASSET_TEXTURE_PIXEL,
    ASSET_TEXTURE_COUNT
};

enum asset_font { ASSET_FONT_SMALL = 0, ASSET_FONT_COUNT };

enum asset_shader {
    ASSET_SHADER_DEFAULT = 0,
    ASSET_SHADER_ATLAS,
    ASSET_SHADER_LINE,
    ASSET_SHADER_COUNT
};

struct assets {
    struct atlas *atlas;
    struct core_texture_region *textures[ASSET_TEXTURE_COUNT];
    struct txt_font *fonts[ASSET_FONT_COUNT];
    Uint32 shaders[ASSET_SHADER_COUNT];
};

bool assets_initialize(struct core *core, struct assets *assets);
void assets_terminate(struct assets *assets);

#endif
