#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"
#include "txt.h"
#include "ui.h"

#define UI_CLOSE_BUTTON_CODEPOINT 8855
#define UI_MINIM_BUTTON_CODEPOINT 8854
#define UI_MAXIM_BUTTON_CODEPOINT 8853

static struct ui_style ui_get_default_style(void)
{
    return (struct ui_style){.background_color = {0.16f, 0.16f, 0.21f, 0},
                             .foreground_color = {0.74f, 0.58f, 0.98f, 0},
                             .font_color = {0.27f, 0.28f, 0.35f, 0},
                             .hover_color = {0.23f, 0.24f, 0.31f, 0},
                             .click_color = {0.21f, 0.22f, 0.30f, 0}};
}

void ui_mk_label(struct ui_element *label, struct assets *assets,
                 struct core *core)
{
    const char *text = label->widget.label.text;
    float text_x = label->rect.x + label->padding;
    float text_y = label->rect.y + label->padding;
    float text_w = label->rect.w - label->padding * 2;
    txt_length(text, text_x, text_y, text_w, assets->fonts[ASSET_FONT_SMALL],
               core);

    // todo handle events
}

void ui_mk_window(struct ui_element *window, struct assets *assets,
                  struct core *core)
{
    struct ui_style style;
    if (window->style == NULL) {
        style = ui_get_default_style();
    } else {
        style = *window->style;
    }

    SDL_FRect pixel_tex_region;
    assets_get_texture_region(assets, ASSET_TEXTURE_PIXEL, &pixel_tex_region);

    // draw window
    core_add_drawing_fill_rect(core, &pixel_tex_region, &window->rect,
                               &style.background_color);

    // draw bar
    int bar_height = txt_get_font_height(assets->fonts[ASSET_FONT_SMALL]);
    SDL_FRect bar_rect = {window->rect.x,
                          window->rect.y + window->rect.h - bar_height,
                          window->rect.w, bar_height};
    core_add_drawing_fill_rect(core, &pixel_tex_region, &bar_rect,
                               &style.foreground_color);
    window->widget.window.row_y = bar_rect.y;

    // draw window border
    core_add_drawing_rect(core, &pixel_tex_region, &window->rect,
                          &style.foreground_color, 1);

    // draw scale button
    SDL_FRect scale_btn_rect = {
        window->rect.x + window->rect.w - bar_height / 2.0f - 2,
        window->rect.y + 2, bar_height / 2.0f, bar_height / 2.0f};
    core_add_drawing_fill_rect(core, &pixel_tex_region, &scale_btn_rect,
                               &style.foreground_color);

    // draw window title on bar
    struct ui_element bar_title_label = {
        .padding = 1,
        .rect = {.x = bar_rect.x, // will be handled by layout func
                 .y = bar_rect.y, // will be handled by layout func
                 // pref value
                 .h = txt_get_font_height(assets->fonts[ASSET_FONT_SMALL]),
                 // pref value
                 .w = 120},
        .widget = {.label = {.text = window->widget.window.title}}};
    ui_mk_label(&bar_title_label, assets, core);

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
