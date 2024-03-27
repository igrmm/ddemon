#include "SDL.h" // IWYU pragma: keep //clangd

#include "../core.h"
#include "app.h"
#include "map.h"
#include "work.h"

static SDL_FPoint offset;
static SDL_FPoint pan_start;
static float scale = 1;

static void work_coord_to_screen(SDL_FPoint work_coord,
                                 SDL_FPoint *screen_coord)
{
    screen_coord->x = (work_coord.x - offset.x) * scale;
    screen_coord->y = (work_coord.y - offset.y) * scale;
}

static void work_coord_from_screen(SDL_FPoint screen_coord,
                                   SDL_FPoint *work_coord)
{
    work_coord->x = screen_coord.x / scale + offset.x;
    work_coord->y = screen_coord.y / scale + offset.y;
}

static void work_rect_to_screen(SDL_FRect rect_work_coord,
                                SDL_FRect *rect_screen_coord)
{
    SDL_FPoint work_coord = {rect_work_coord.x, rect_work_coord.y};
    SDL_FPoint screen_coord = {0, 0};
    work_coord_to_screen(work_coord, &screen_coord);
    rect_screen_coord->x = screen_coord.x;
    rect_screen_coord->y = screen_coord.y;
    rect_screen_coord->w = rect_work_coord.w * scale;
    rect_screen_coord->h = rect_work_coord.h * scale;
}

static void work_rect_from_screen(SDL_FRect rect_screen_coord,
                                  SDL_FRect *rect_work_coord)
{
    SDL_FPoint screen_coord = {rect_screen_coord.x, rect_screen_coord.y};
    SDL_FPoint work_coord = {0, 0};
    work_coord_from_screen(screen_coord, &work_coord);
    rect_work_coord->x = work_coord.x;
    rect_work_coord->y = work_coord.y;
    rect_work_coord->w = rect_screen_coord.w / scale;
    rect_work_coord->h = rect_screen_coord.h / scale;
}

void work_render(struct app *app)
{
    SDL_Renderer *renderer = app->core->renderer;
    int tile_size = MAP_TILE_SIZE;

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    // render tiles
    SDL_Rect src_rect = {0, 0, tile_size, tile_size};
    SDL_FRect dst_rect = {0, 0, tile_size, tile_size};
    SDL_FPoint work_coord = {0, 0};
    SDL_FPoint screen_coord = {0, 0};
    struct map_tile_layer *tile_layer = NULL;

    for (int i = 0; i < MAP_TILES_X_MAX; i++) {
        for (int j = 0; j < MAP_TILES_Y_MAX; j++) {
            for (int l = 0; l < MAP_TILE_LAYERS_MAX; l++) {
                tile_layer = &app->map->tiles[i][j].layers[l];
                if (tile_layer->enabled == 0)
                    continue;
                src_rect.x = tile_layer->tileset_index_x * tile_size;
                src_rect.y = tile_layer->tileset_index_y * tile_size;
                // make coordinate conversion
                work_coord.x = i * tile_size;
                work_coord.y = j * tile_size;
                work_coord_to_screen(work_coord, &screen_coord);
                dst_rect.x = screen_coord.x;
                dst_rect.y = screen_coord.y;
                dst_rect.w = dst_rect.h = tile_size * scale;

                SDL_RenderCopyF(renderer, app->tileset_texture, &src_rect,
                                &dst_rect);
            }
        }
    }

    // render grid
    if (app->show_grid) {
        int cols = MAP_TILES_X_MAX;
        SDL_FPoint col0_work, col0_screen, col1_work, col1_screen;
        for (int col = 0; col <= cols; col++) {
            col0_work.x = col * tile_size;
            col0_work.y = 0;
            col1_work.x = col * tile_size;
            col1_work.y = MAP_TILES_Y_MAX * tile_size;

            work_coord_to_screen(col0_work, &col0_screen);
            work_coord_to_screen(col1_work, &col1_screen);

            SDL_RenderDrawLineF(renderer, col0_screen.x, col0_screen.y,
                                col1_screen.x, col1_screen.y);
        }

        int rows = MAP_TILES_Y_MAX;
        SDL_FPoint row0_work, row0_screen, row1_work, row1_screen;
        for (int row = 0; row <= rows; row++) {
            row0_work.x = 0;
            row0_work.y = row * tile_size;
            row1_work.x = MAP_TILES_X_MAX * tile_size;
            row1_work.y = row * tile_size;

            work_coord_to_screen(row0_work, &row0_screen);
            work_coord_to_screen(row1_work, &row1_screen);

            SDL_RenderDrawLineF(renderer, row0_screen.x, row0_screen.y,
                                row1_screen.x, row1_screen.y);
        }
    }
}
