#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"
#include "txt.h"

struct txt_font {
    struct asset_atlas *atlas;
    int glyph_atlas_indexes[TXT_UNICODE_MAX];
};

struct txt_codepoint_cache {
    SDL_bool codepoints[TXT_UNICODE_MAX];
    size_t count;
};

int txt_get_codepoint(Uint32 *codepoint, const char **iterator)
{
    unsigned char byte = **iterator;

    // check if the character is ascii
    if ((byte & 0x80) == 0) {
        *codepoint = byte;
        return 0;
    }

    // check how many bytes are encoded
    int num_bytes = 0;
    if ((byte & 0xE0) == 0xC0) {
        num_bytes = 2;
    } else if ((byte & 0xF0) == 0xE0) {
        num_bytes = 3;
    } else if ((byte & 0xF8) == 0xF0) {
        num_bytes = 4;
    } else {
        SDL_Log("Error getting codepoint: invalid utf8.");
        return -1;
    }

    for (int i = 0; i < num_bytes; i++) {
        /**
         * If this is the first byte of the encoding, skip first bits
         * accordinly to num_bytes, else add next bytes (with the first 2 bits
         * skipped) of the encoding to the codepoint
         */
        if (i == 0) {
            *codepoint = (byte & ((1 << (7 - num_bytes)) - 1));
        } else {
            *codepoint <<= 6;
            *codepoint |= (byte & 63);
        }

        // don't advance the iterator if this is the last iteration of the loop
        if (i < num_bytes - 1) {
            (*iterator)++;
            byte = **iterator;
        }
    }

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

        int index = font->glyph_atlas_indexes[codepoint];
        SDL_FRect glyph_region;
        assets_get_texture_region(font->atlas, index, &glyph_region);
        src_rect = (SDL_FRect){0, 0, glyph_region.w, glyph_region.h};
        dst_rect = (SDL_FRect){0, y, glyph_region.w, glyph_region.h};
        dst_rect.x = x + cursor_x;
        core_add_drawing_tex(core, &glyph_region, &src_rect, &dst_rect);
        cursor_x += glyph_region.w + 1;
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

void txt_set_glyph_atlas_index(struct txt_font *font, Uint32 codepoint,
                               Uint32 index)
{
    font->glyph_atlas_indexes[codepoint] = index;
}
