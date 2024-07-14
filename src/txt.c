#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"
#include "txt.h"

#define TXT_CODEPOINT_BUFSIZ 1024

struct txt_font {
    struct asset_atlas *atlas;
    int texture_region_ids[TXT_UNICODE_MAX];
};

struct txt_codepoint_cache {
    SDL_bool codepoints[TXT_UNICODE_MAX];
    size_t count;
};

int txt_decode_codepoints(Uint32 *cp, size_t cp_maxlen, const char *str)
{
    // ASCII ONLY FOR NOW
    unsigned char byte1;
    size_t str_len = SDL_strlen(str);
    size_t cp_i = 0;
    for (size_t str_i = 0; str_i < str_len; str_i++) {
        byte1 = str[str_i];
        if (byte1 > 0x7f || cp_i >= cp_maxlen)
            return -1;
        cp[cp_i] = byte1;
        cp_i++;
    }

    return 0;
}

int txt_cache_codepoints(struct txt_codepoint_cache *cache, const char *str)
{
    // convert utf8 string to array of codepoints (decimal, int)
    Uint32 buffer[TXT_CODEPOINT_BUFSIZ] = {0};
    size_t bufsiz = SDL_arraysize(buffer);
    if (txt_decode_codepoints(buffer, bufsiz, str) < 0)
        return -1;

    // this loop will flag unique codepoints true
    Uint32 codepoint;
    for (size_t i = 0; i < bufsiz; i++) {
        codepoint = buffer[i];
        if (cache->codepoints[codepoint] == SDL_FALSE && codepoint > 0) {
            cache->codepoints[codepoint] = SDL_TRUE;
            cache->count++;
        }
    }

    return 0;
}

struct txt_codepoint_cache *txt_create_codepoint_cache(void)
{
    struct txt_codepoint_cache *cache =
        SDL_calloc(1, sizeof(struct txt_codepoint_cache));
    return cache;
}

struct txt_font *txt_create_font(struct asset_atlas *atlas)
{
    struct txt_font *font = SDL_calloc(1, sizeof(struct txt_font));
    if (font == NULL)
        return font;
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
    Uint32 codepoints[TXT_CODEPOINT_BUFSIZ] = {0};
    SDL_FRect src_rect, dst_rect;
    int cursor_x = 0;

    if (txt_decode_codepoints(codepoints, TXT_CODEPOINT_BUFSIZ, str) < 0)
        return -1;

    for (int i = 0; i < TXT_CODEPOINT_BUFSIZ; i++) {
        if (codepoints[i] == 0)
            break;

        // TODO hardcoded: size of "space"
        if (codepoints[i] == ' ') {
            cursor_x += 32;
            continue;
        }

        int texture_region_id = font->texture_region_ids[codepoints[i]];
        SDL_FRect texture_region;
        assets_atlas_get_texture_region(font->atlas, texture_region_id,
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
