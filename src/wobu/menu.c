#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#include "../core.h"
#include "app.h"
#include "menu.h"

void menu_window(struct app *app)
{
    struct nk_context *nk_ctx = app->nk_ctx;
    int h = 25;
    int w = app->core->window_width;
    int x = 0;
    int y = 0;

    if (nk_begin(nk_ctx, "menu", nk_rect(x, y, w, h), NK_WINDOW_NO_SCROLLBAR)) {
        nk_menubar_begin(nk_ctx);
        nk_layout_row_static(nk_ctx, h, 45, 2);

        if (nk_menu_begin_label(nk_ctx, "file", NK_TEXT_ALIGN_LEFT,
                                nk_vec2(100, 100))) {
            nk_layout_row_dynamic(nk_ctx, h, 1);

            if (nk_menu_item_label(nk_ctx, "new", NK_TEXT_LEFT)) {
                // todo
            }

            if (nk_menu_item_label(nk_ctx, "save", NK_TEXT_LEFT)) {
                // todo
            }

            if (nk_menu_item_label(nk_ctx, "load", NK_TEXT_LEFT)) {
                // todo
            }
            nk_menu_end(nk_ctx);
        }

        if (nk_menu_begin_label(nk_ctx, "view", NK_TEXT_ALIGN_LEFT,
                                nk_vec2(100, 120))) {
            nk_layout_row_dynamic(nk_ctx, h, 1);

            // grid
            char grid_title[] = "*grid";
            if (!app->show_grid) {
                SDL_snprintf(grid_title, sizeof grid_title, " grid");
            }
            if (nk_menu_item_label(nk_ctx, grid_title, NK_TEXT_LEFT)) {
                app->show_grid = !app->show_grid;
            }

            // tools window
            char toolsw_title[] = "*tools";
            if (!app->show_tool_window) {
                SDL_snprintf(toolsw_title, sizeof toolsw_title, " tools");
            }
            if (nk_menu_item_label(nk_ctx, toolsw_title, NK_TEXT_LEFT)) {
                app->show_tool_window = !app->show_tool_window;
            }

            // tileset window
            char tilesetw_title[] = "*tileset";
            if (!app->show_pick_window) {
                SDL_snprintf(tilesetw_title, sizeof tilesetw_title, " tileset");
            }
            if (nk_menu_item_label(nk_ctx, tilesetw_title, NK_TEXT_LEFT)) {
                app->show_pick_window = !app->show_pick_window;
            }

            // properties window
            // char propertiesw_title[] = "*properties";
            // if (!app->show_propertiesw)
            //    SDL_snprintf(propertiesw_title, sizeof propertiesw_title,
            //                 " properties");

            // if (nk_menu_item_label(nk_ctx, propertiesw_title, NK_TEXT_LEFT))
            //     app->show_propertiesw = !app->show_propertiesw;

            nk_menu_end(nk_ctx);
        }
        nk_menubar_end(nk_ctx);
    }
    nk_end(nk_ctx);
}
