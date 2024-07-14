#ifndef TXT_H
#define TXT_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"

#define TXT_UNICODE_MAX 1112064

struct txt_font;
struct txt_codepoint_cache;

/**
 * This function decodes codepoints of given string into decimal values and
 * store in cp, an array of Uint32. Essentially this converts a given utf8
 * string into an array of ints representing the unicode codepoints as decimal
 * values.
 *
 */
int txt_decode_codepoints(Uint32 *cp, size_t cp_maxlen, const char *str);

/**
 * Caches unique codepoints as flags (true or false) of given string in an array
 * of booleans to be accessed by index as codepoint decimal value.
 *
 */
int txt_cache_codepoints(struct txt_codepoint_cache *cache, const char *str);

struct txt_codepoint_cache *txt_create_codepoint_cache(void);
struct txt_font *txt_create_font(struct asset_atlas *atlas);
void txt_destroy_font(struct txt_font *font);

/**
 * Draw text on screen made of str string. The text will be with maximum of
 * TXT_CODEPOINT_BUFSIZ codepoints.
 *
 */
int txt(const char *str, float x, float y, struct txt_font *font,
        struct core *core);
SDL_bool txt_is_codepoint_cached(struct txt_codepoint_cache *cache,
                                 Uint32 codepoint);
void txt_set_glyph(struct txt_font *font, Uint32 codepoint,
                   Uint32 texture_region_id);

#endif
