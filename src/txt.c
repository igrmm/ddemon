#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"
#include "txt.h"

struct txt_font {
    struct asset_atlas *atlas;
    int texture_region_ids[TXT_UNICODE_MAX];
};

struct txt_codepoint_cache {
    SDL_bool codepoints[TXT_UNICODE_MAX];
    size_t count;
};

int txt_get_codepoint(Uint32 *codepoint, const char **iterator)
{
    // ASCII ONLY FOR NOW
    unsigned char byte1 = **iterator;
    if (byte1 > 0x7f) {
        SDL_Log("Error decoding non ascii unicode: not implemented.");
        return -1;
    }
    *codepoint = byte1;
    return 0;
}

int txt_cache_codepoints(struct txt_codepoint_cache *cache, const char *str)
{
    Uint32 codepoint;
    const char *iterator = str;
    while (*iterator != '\0') {
        if (txt_get_codepoint(&codepoint, &iterator) != 0)
            return -1;
        if (cache->codepoints[codepoint] == SDL_FALSE && codepoint > 0) {
            cache->codepoints[codepoint] = SDL_TRUE;
            cache->count++;
        }
        iterator++;
    }
    return 0;
}

struct txt_codepoint_cache *txt_create_codepoint_cache(void)
{
    struct txt_codepoint_cache *cache =
        SDL_calloc(1, sizeof(struct txt_codepoint_cache));
    if (cache == NULL)
        SDL_Log("Error creating codepoint cache: calloc failed.");
    return cache;
}

struct txt_font *txt_create_font(struct asset_atlas *atlas)
{
    struct txt_font *font = SDL_calloc(1, sizeof(struct txt_font));
    if (font == NULL) {
        SDL_Log("Error creating txt_font: calloc failed.");
        return font;
    }
    font->atlas = atlas;
    return font;
}

void txt_destroy_font(struct txt_font *font)
{
    if (font != NULL)
        SDL_free(font);
}

int txt(const char *str, float x, float y, struct txt_font *font,
        struct core *core)
{
    SDL_FRect src_rect, dst_rect;
    int cursor_x = 0;
    Uint32 codepoint;
    const char *iterator = str;

    while (*iterator != '\0') {
        if (txt_get_codepoint(&codepoint, &iterator) != 0)
            return -1;

        iterator++;

        // TODO hardcoded: size of "space"
        if (codepoint == ' ') {
            cursor_x += 32;
            continue;
        }

        int texture_region_id = font->texture_region_ids[codepoint];
        SDL_FRect texture_region;
        assets_get_texture_region(font->atlas, texture_region_id,
                                  &texture_region);
        src_rect = (SDL_FRect){0, 0, texture_region.w, texture_region.h};
        dst_rect = (SDL_FRect){0, y, texture_region.w, texture_region.h};
        dst_rect.x = x + cursor_x;
        core_add_drawing_tex(core, &texture_region, &src_rect, &dst_rect);
        cursor_x += texture_region.w + 1;
    }

    return 0;
}

SDL_bool txt_is_codepoint_cached(struct txt_codepoint_cache *cache,
                                 Uint32 codepoint)
{
    size_t cache_size = SDL_arraysize(cache->codepoints);
    if (codepoint > 0 && codepoint < cache_size)
        return cache->codepoints[codepoint];
    return SDL_FALSE;
}

void txt_set_glyph(struct txt_font *font, Uint32 codepoint,
                   Uint32 texture_region_id)
{
    font->texture_region_ids[codepoint] = texture_region_id;
}
