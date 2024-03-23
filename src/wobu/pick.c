#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#include "app.h"
#include "pick.h"

static void pick_tile_from_tileset_on_click(struct app *app,
                                            struct nk_context *nk_ctx,
                                            struct nk_rect tileset_rect)
{
    if (nk_input_is_mouse_click_in_rect(&nk_ctx->input, NK_BUTTON_LEFT,
                                        tileset_rect)) {
        int tile_size = 32;
        // int tile_size = app->map->tile_size;

        // sets the x,y index of selected tile in tileset
        app->selected_tileset_index.x =
            (nk_ctx->input.mouse.pos.x - tileset_rect.x +
             nk_ctx->current->scrollbar.x) /
            tile_size;
        app->selected_tileset_index.y =
            (nk_ctx->input.mouse.pos.y - tileset_rect.y +
             nk_ctx->current->scrollbar.y) /
            tile_size;
    }
}

static void pick_render(struct app *app, struct nk_context *nk_ctx,
                        struct nk_command_buffer *canvas,
                        struct nk_rect tileset_rect)
{
    struct nk_image tileset_image = nk_image_ptr(app->tileset_texture);
    int tile_size = 32;
    // int tile_size = app->map->tile_size;

    // draw tileset
    nk_image(nk_ctx, tileset_image);

    // draw grid - column lines
    int cols = tileset_rect.w + nk_ctx->current->scrollbar.x;
    for (int col = 0; col <= cols; col += tile_size) {
        float col0_x = tileset_rect.x + col - nk_ctx->current->scrollbar.x;
        float col0_y = tileset_rect.y;
        float col1_x = tileset_rect.x + col - nk_ctx->current->scrollbar.x;
        float col1_y = tileset_rect.y + tileset_rect.h;

        nk_stroke_line(canvas, col0_x, col0_y, col1_x, col1_y, 1.0f, GREY);
    }

    // draw grid - row lines
    int rows = tileset_rect.h + nk_ctx->current->scrollbar.y;
    for (int row = 0; row <= rows; row += tile_size) {
        float row0_x = tileset_rect.x;
        float row0_y = tileset_rect.y + row - nk_ctx->current->scrollbar.y;
        float row1_x = tileset_rect.x + tileset_rect.w;
        float row1_y = tileset_rect.y + row - nk_ctx->current->scrollbar.y;

        nk_stroke_line(canvas, row0_x, row0_y, row1_x, row1_y, 1.0f, GREY);
    }

    // draws semi transparent green rect on top of selected tile
    if (app->selected_tileset_index.x >= 0 &&
        app->selected_tileset_index.y >= 0) {
        float selected_tileset_x = app->selected_tileset_index.x * tile_size;
        float selected_tileset_y = app->selected_tileset_index.y * tile_size;
        float green_rect_x =
            tileset_rect.x + selected_tileset_x - nk_ctx->current->scrollbar.x;
        float green_rect_y =
            tileset_rect.y + selected_tileset_y - nk_ctx->current->scrollbar.y;

        nk_fill_rect(canvas,
                     nk_rect(green_rect_x, green_rect_y, tile_size, tile_size),
                     0, nk_rgba(GREEN.r, GREEN.g, GREEN.b, 150));
    }
}

void pick_window(struct app *app)
{
    struct nk_context *nk_ctx = app->nk_ctx;
    int window_flags = app->window_flags;
    int tileset_width, tileset_height;
    SDL_QueryTexture(app->tileset_texture, NULL, NULL, &tileset_width,
                     &tileset_height);

    // backup padding
    struct nk_vec2 padding_bkp = nk_ctx->style.window.padding;
    nk_ctx->style.window.padding = nk_vec2(0, 0);

    if (nk_begin(nk_ctx, "tileset", nk_rect(20, 40, 200, 200), window_flags)) {
        nk_layout_row_static(nk_ctx, tileset_height, tileset_width, 1);

        struct nk_command_buffer *canvas = nk_window_get_canvas(nk_ctx);

        float w = SDL_min(tileset_width, canvas->clip.w);
        float h = SDL_min(tileset_height, canvas->clip.h);
        struct nk_rect tileset_rect =
            nk_rect(canvas->clip.x, canvas->clip.y, w, h);

        pick_tile_from_tileset_on_click(app, nk_ctx, tileset_rect);
        pick_render(app, nk_ctx, canvas, tileset_rect);
    } else {
        app->show_pick_window = 0;
    }
    nk_end(nk_ctx);

    // restore padding
    nk_ctx->style.window.padding = padding_bkp;
}
