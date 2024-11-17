#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"
#include "txt.h"
#include "ui.h"

#define UI_CLOSE_BUTTON_CODEPOINT 8855
#define UI_MINIM_BUTTON_CODEPOINT 8854
#define UI_MAXIM_BUTTON_CODEPOINT 8853

static struct ui_style style = {.background_color = {0.16f, 0.16f, 0.21f, 0},
                                .foreground_color = {0.74f, 0.58f, 0.98f, 0},
                                .font_color = {0.27f, 0.28f, 0.35f, 0},
                                .hover_color = {0.23f, 0.24f, 0.31f, 0},
                                .click_color = {0.21f, 0.22f, 0.30f, 0}};

static struct txt_font *font = NULL;

void ui_set_font(struct txt_font *in_font) { font = in_font; }

void ui_set_style(struct ui_style *in_style) { style = *in_style; }

struct ui_style ui_get_style(void) { return style; }

void ui_mk_button(struct ui_element *button, struct assets *assets,
                  struct core *core)
{
    if (font == NULL)
        return;

    SDL_FRect *tex_region = &button->widget.button.tex_region;
    SDL_FRect src_rect = {.w = tex_region->w, .h = tex_region->h};
    // todo: add text offset to dst_rect x position
    SDL_FRect dst_rect = {.x = button->rect.x + button->padding,
                          .y = button->rect.y + button->padding,
                          .w = tex_region->w,
                          .h = tex_region->h};
    core_add_drawing_color_tex(core, tex_region, &src_rect, &dst_rect,
                               &style.font_color);
}

void ui_mk_label(struct ui_element *label, struct assets *assets,
                 struct core *core)
{
    if (font == NULL)
        return;

    const char *text = label->widget.label.text;
    float text_x = label->rect.x + label->padding;
    float text_y = label->rect.y + label->padding;
    float text_w = label->rect.w - label->padding * 2;
    struct core_color *text_color = &style.font_color;
    txt_length(text, text_x, text_y, text_w, text_color, font, core);

    // todo handle events
}

void ui_mk_window(struct ui_element *window, struct assets *assets,
                  struct core *core)
{
    if (font == NULL)
        return;

    SDL_FRect pixel_tex_region;
    assets_get_texture_region(assets, ASSET_TEXTURE_PIXEL, &pixel_tex_region);

    // draw window
    core_add_drawing_fill_rect(core, &pixel_tex_region, &window->rect,
                               &style.background_color);

    // draw bar
    int bar_height = txt_get_font_height(font);
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
                 .h = txt_get_font_height(font),
                 // pref value
                 .w = 120},
        .widget = {.label = {.text = window->widget.window.title}}};
    ui_mk_label(&bar_title_label, assets, core);

    SDL_FRect tex_region;

    // draw close button
    txt_get_glyph_region(&tex_region, UI_CLOSE_BUTTON_CODEPOINT, font);
    struct ui_element close_btn = {
        .padding = 1,
        .rect =
            {
                .x = bar_rect.x + bar_rect.w - tex_region.w - 2,
                .y = bar_rect.y,
            },
        .widget = {.button = {.tex_region = tex_region}}};
    ui_mk_button(&close_btn, assets, core);

    // draw maximize button
    txt_get_glyph_region(&tex_region, UI_MAXIM_BUTTON_CODEPOINT, font);
    struct ui_element maxim_btn = {
        .padding = 1,
        .rect =
            {
                .x = close_btn.rect.x - tex_region.w - 1,
                .y = bar_rect.y,
            },
        .widget = {.button = {.tex_region = tex_region}}};
    ui_mk_button(&maxim_btn, assets, core);

    // draw minimize button
    txt_get_glyph_region(&tex_region, UI_MINIM_BUTTON_CODEPOINT, font);
    struct ui_element minim_btn = {
        .padding = 1,
        .rect =
            {
                .x = maxim_btn.rect.x - tex_region.w - 1,
                .y = bar_rect.y,
            },
        .widget = {.button = {.tex_region = tex_region}}};
    ui_mk_button(&minim_btn, assets, core);
}
