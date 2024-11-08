#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"
#include "txt.h"
#include "ui.h"

#define UI_CLOSE_BUTTON_CODEPOINT 8855
#define UI_MINIM_BUTTON_CODEPOINT 8854
#define UI_MAXIM_BUTTON_CODEPOINT 8853

void ui_mk_window(struct ui_window *ui_window, struct assets *assets,
                  struct core *core)
{
    SDL_FRect pixel_tex_region;
    assets_get_texture_region(assets, ASSET_TEXTURE_PIXEL, &pixel_tex_region);

    // draw window
    core_add_drawing_fill_rect(core, &pixel_tex_region, &ui_window->rect,
                               &ui_window->bg_color);

    // draw bar
    int bar_height = txt_get_font_height(assets->fonts[ASSET_FONT_SMALL]);
    SDL_FRect bar_rect = {ui_window->rect.x,
                          ui_window->rect.y + ui_window->rect.h - bar_height,
                          ui_window->rect.w, bar_height};
    core_add_drawing_fill_rect(core, &pixel_tex_region, &bar_rect,
                               &ui_window->fg_color);

    // draw window border
    core_add_drawing_rect(core, &pixel_tex_region, &ui_window->rect,
                          &ui_window->fg_color, 1);

    // draw scale button
    SDL_FRect scale_btn_rect = {
        ui_window->rect.x + ui_window->rect.w - bar_height / 2.0f - 2,
        ui_window->rect.y + 2, bar_height / 2.0f, bar_height / 2.0f};
    core_add_drawing_fill_rect(core, &pixel_tex_region, &scale_btn_rect,
                               &ui_window->fg_color);

    // draw window title on bar
    txt(ui_window->title, bar_rect.x + 1, bar_rect.y + 1,
        assets->fonts[ASSET_FONT_SMALL], core);

    SDL_FRect btn_region, src_rect;

    // draw close button
    txt_get_glyph_region(&btn_region, UI_CLOSE_BUTTON_CODEPOINT,
                         assets->fonts[ASSET_FONT_SMALL]);
    src_rect = (SDL_FRect){0, 0, btn_region.w, btn_region.h};
    SDL_FRect close_btn_rect = {bar_rect.x + bar_rect.w - btn_region.w - 1,
                                bar_rect.y + 1, btn_region.w, btn_region.h};
    core_add_drawing_tex(core, &btn_region, &src_rect, &close_btn_rect);

    // draw maximize button
    txt_get_glyph_region(&btn_region, UI_MAXIM_BUTTON_CODEPOINT,
                         assets->fonts[ASSET_FONT_SMALL]);
    src_rect = (SDL_FRect){0, 0, btn_region.w, btn_region.h};
    SDL_FRect maxim_btn_rect = {close_btn_rect.x - btn_region.w - 1,
                                bar_rect.y + 1, btn_region.w, btn_region.h};
    core_add_drawing_tex(core, &btn_region, &src_rect, &maxim_btn_rect);

    // draw minimize button
    txt_get_glyph_region(&btn_region, UI_MINIM_BUTTON_CODEPOINT,
                         assets->fonts[ASSET_FONT_SMALL]);
    src_rect = (SDL_FRect){0, 0, btn_region.w, btn_region.h};
    SDL_FRect minim_btn_rect = {maxim_btn_rect.x - btn_region.w - 1,
                                bar_rect.y + 1, btn_region.w, btn_region.h};
    core_add_drawing_tex(core, &btn_region, &src_rect, &minim_btn_rect);
}
