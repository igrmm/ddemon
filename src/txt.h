#ifndef TXT_H
#define TXT_H

#include <SDL3/SDL.h>

#include "assets.h"
#include "core.h"

#define TXT_UNICODE_MAX 1112064

struct atlas;

struct txt_font;
struct txt_codepoint_cache;

/**
 * This function decodes the character of some string pointed by the iterator
 * into a unicode codepoint, a decimal value. If the character is encoded in
 * more than one byte, the function will advance the iterator until the decoding
 * is done, or fail.
 *
 * Example:
 * const char utf8[] = "this is a string";
 * const char *iterator = utf8;
 * while(*iterator != '\0') {
 *     Uint32 codepoint;
 *     if(!txt_get_codepoint(&codepoint, &iterator))
 *         break;
 *     iterator++;
 * }
 *
 * \param codepoint a pointer filled in with the unicode codepoint, a decimal
 * value representing the character.
 * \param iterator a double pointer pointing to a character of a utf8 string
 * that will be decoded to a unicode codepoint.
 * \returns true on success or false on error.
 *
 */
bool txt_get_codepoint(Uint32 *codepoint, const char **iterator);

/**
 * Caches unique codepoints as flags (true or false) of given string in an array
 * of booleans to be accessed by index as codepoint decimal value.
 *
 */
bool txt_cache_codepoints(struct txt_codepoint_cache *cache, const char *str);

struct txt_codepoint_cache *txt_create_codepoint_cache(void);
struct txt_font *txt_create_font(int height, struct atlas *atlas);
void txt_destroy_font(struct txt_font *font);
void txt_get_string_rect_size(const char *str, float *width, float *height,
                              struct txt_font *font);

/**
 * Draw text on screen made of "str" string with a maximum "length" line length
 * in pixels.
 *
 */
bool txt_length(const char *str, float x, float y, float length,
                struct core_color *color, struct txt_font *font,
                struct core *core);

/**
 * Draw text on screen made of "str" string with no length limit.
 *
 */
bool txt(const char *str, float x, float y, struct txt_font *font,
         struct core *core);
bool txt_is_codepoint_cached(struct txt_codepoint_cache *cache,
                             Uint32 codepoint);
void txt_set_glyph_atlas_index(struct txt_font *font, Uint32 codepoint,
                               Uint32 index);
void txt_set_font_advance_x(float advance_x, struct txt_font *font);
int txt_get_font_height(struct txt_font *font);
void txt_get_glyph_region(SDL_FRect *region, Uint32 codepoint,
                          struct txt_font *font);

#endif
