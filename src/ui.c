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

static void ui_compute_button_size(int height, struct ui_element *button)
{
    int padding_num = 0;

    // check if button have text or image
    if (button->data.button.text_width > 0 ||
        button->data.button.tex_region.w > 0)
        padding_num = 2;

    // check if button have both text and image
    if (button->data.button.text_width > 0 &&
        button->data.button.tex_region.w > 0)
        padding_num = 3;

    button->rect.w = button->data.button.text_width +
                     button->data.button.tex_region.w +
                     padding_num * button->padding;
    button->rect.h = height;
}

static void ui_compute_label_size(int height, struct ui_element *label)
{
    if (label->data.label.text_width > 0)
        label->rect.w = label->data.label.text_width + 2 * label->padding;
    label->rect.h = height;
}

void ui_layout_row(struct ui_element *window, int height,
                   struct ui_element *elements[], int element_count)
{
    // calculate element rect, get amount of "free width" and growable elements
    int row_free_width = window->rect.w, growable_element_count = 0;
    for (int i = 0; i < element_count; i++) {
        struct ui_element *element = elements[i];
        if (element == NULL) {
            growable_element_count++;
        } else if (element->type == UI_TYPE_BUTTON) {
            ui_compute_button_size(height, element);
            row_free_width -= element->rect.w;
        } else if (element->type == UI_TYPE_LABEL) {
            ui_compute_label_size(height, element);
            row_free_width -= element->rect.w;
        } else {
            // NOT IMPLEMENTED
        }
    }

    int grow_width = 0;
    if (growable_element_count > 0)
        grow_width = row_free_width / growable_element_count;

    // layout
    int x = window->rect.x;
    int y = window->data.window.row_y - height;
    for (int i = 0; i < element_count; i++) {
        struct ui_element *element = elements[i];
        if (element == NULL) {
            x += grow_width;
        } else {
            element->rect.x = x;
            element->rect.y = y;
            x += element->rect.w;
        }
    }
    window->data.window.row_y = y;
}

void ui_mk_button(struct ui_element *button, struct assets *assets,
                  struct core *core)
{
    if (font == NULL)
        return;

    // todo: exit function if element isnt inside window

    SDL_FRect *tex_region = &button->data.button.tex_region;
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

    // todo: exit function if element isnt inside window

    const char *text = label->data.label.text;
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
    window->data.window.row_y = window->rect.y + window->rect.h;

    // draw window border
    core_add_drawing_rect(core, &pixel_tex_region, &window->rect,
                          &style.foreground_color, 1);

    // draw scale button
    SDL_FRect scale_btn_rect = {
        window->rect.x + window->rect.w - bar_height / 2.0f - 2,
        window->rect.y + 2, bar_height / 2.0f, bar_height / 2.0f};
    core_add_drawing_fill_rect(core, &pixel_tex_region, &scale_btn_rect,
                               &style.foreground_color);

    SDL_FRect tex_region;

    struct ui_element bar_title_label = {
        .padding = 1,
        .type = UI_TYPE_LABEL,
        .data = {
            .label = {.text = window->data.window.title, .text_width = 120}}};

    txt_get_glyph_region(&tex_region, UI_MINIM_BUTTON_CODEPOINT, font);
    struct ui_element minim_btn = {
        .padding = 1,
        .type = UI_TYPE_BUTTON,
        .data = {.button = {.tex_region = tex_region}}};

    txt_get_glyph_region(&tex_region, UI_MAXIM_BUTTON_CODEPOINT, font);
    struct ui_element maxim_btn = {
        .padding = 1,
        .type = UI_TYPE_BUTTON,
        .data = {.button = {.tex_region = tex_region}}};

    txt_get_glyph_region(&tex_region, UI_CLOSE_BUTTON_CODEPOINT, font);
    struct ui_element close_btn = {
        .padding = 1,
        .type = UI_TYPE_BUTTON,
        .data = {.button = {.tex_region = tex_region}}};

    struct ui_element *elements[] = {&bar_title_label, NULL, &minim_btn,
                                     &maxim_btn, &close_btn};
    ui_layout_row(window, bar_height, elements, SDL_arraysize(elements));

    ui_mk_label(&bar_title_label, assets, core);
    ui_mk_button(&close_btn, assets, core);
    ui_mk_button(&maxim_btn, assets, core);
    ui_mk_button(&minim_btn, assets, core);
}
