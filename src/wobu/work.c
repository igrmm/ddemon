#include "SDL.h" // IWYU pragma: keep //clangd

#include "../core.h"
#include "app.h"
#include "map.h"
#include "work.h"

static SDL_FPoint work_offset;
static SDL_FPoint work_pan_start_point;
static float work_scale = 1;

static void work_state_zoom(SDL_Event *event, struct app *app);
static void work_state_pan_start(SDL_Event *event, struct app *app);
static void work_state_pan(SDL_Event *event, struct app *app);
static void work_state_paint(SDL_Event *event, struct app *app);
static void work_state_paint_motion(SDL_Event *event, struct app *app);
static void work_state_erase(SDL_Event *event, struct app *app);

static void (*work_state_table[WORK_STATE_TOTAL])(SDL_Event *event,
                                                  struct app *app) = {
    // clang-format off
    work_state_zoom,
    work_state_pan_start,
    work_state_pan,
    work_state_paint,
    work_state_paint_motion,
    work_state_erase,
    NULL,
    NULL
    // clang-format on
};

static void work_coord_to_screen(SDL_FPoint work_coord,
                                 SDL_FPoint *screen_coord)
{
    screen_coord->x = (work_coord.x - work_offset.x) * work_scale;
    screen_coord->y = (work_coord.y - work_offset.y) * work_scale;
}

static void work_coord_from_screen(SDL_FPoint screen_coord,
                                   SDL_FPoint *work_coord)
{
    work_coord->x = screen_coord.x / work_scale + work_offset.x;
    work_coord->y = screen_coord.y / work_scale + work_offset.y;
}

static void work_rect_to_screen(SDL_FRect rect_work_coord,
                                SDL_FRect *rect_screen_coord)
{
    SDL_FPoint work_coord = {rect_work_coord.x, rect_work_coord.y};
    SDL_FPoint screen_coord = {0, 0};
    work_coord_to_screen(work_coord, &screen_coord);
    rect_screen_coord->x = screen_coord.x;
    rect_screen_coord->y = screen_coord.y;
    rect_screen_coord->w = rect_work_coord.w * work_scale;
    rect_screen_coord->h = rect_work_coord.h * work_scale;
}

static void work_rect_from_screen(SDL_FRect rect_screen_coord,
                                  SDL_FRect *rect_work_coord)
{
    SDL_FPoint screen_coord = {rect_screen_coord.x, rect_screen_coord.y};
    SDL_FPoint work_coord = {0, 0};
    work_coord_from_screen(screen_coord, &work_coord);
    rect_work_coord->x = work_coord.x;
    rect_work_coord->y = work_coord.y;
    rect_work_coord->w = rect_screen_coord.w / work_scale;
    rect_work_coord->h = rect_screen_coord.h / work_scale;
}

static int work_get_tile_index_on_mouse(SDL_FPoint mouse_screen_coord,
                                        SDL_Point *tile_index)
{
    SDL_FPoint mouse_work_coord;
    work_coord_from_screen(mouse_screen_coord, &mouse_work_coord);

    int map_width = MAP_TILES_X_MAX * MAP_TILE_SIZE;
    int map_height = MAP_TILES_Y_MAX * MAP_TILE_SIZE;
    SDL_FRect map_rect = {0, 0, map_width, map_height};

    if (SDL_PointInFRect(&mouse_work_coord, &map_rect)) {
        tile_index->x = mouse_work_coord.x / MAP_TILE_SIZE;
        tile_index->y = mouse_work_coord.y / MAP_TILE_SIZE;
        return 0;
    }
    return -1;
}

static void work_set_tile_layer_on_mouse(SDL_FPoint mouse_screen_coord,
                                         struct map *map, int layer,
                                         struct map_tile_layer tile_layer)
{
    SDL_Point tile_index;
    if (work_get_tile_index_on_mouse(mouse_screen_coord, &tile_index) == 0) {
        map->tiles[tile_index.x][tile_index.y].layers[layer] = tile_layer;
    }
}

static void work_state_zoom(SDL_Event *event, struct app *app)
{
    SDL_FPoint mouse_screen_coord = {event->wheel.mouseX, event->wheel.mouseY};
    SDL_FPoint mouse_work_coord_before_zoom = {0, 0};

    work_coord_from_screen(mouse_screen_coord, &mouse_work_coord_before_zoom);

    if (event->wheel.y > 0) {
        work_scale *= 1.1f;

    } else if (event->wheel.y < 0) {
        work_scale *= 0.9f;
    }

    SDL_FPoint mouse_work_coord_after_zoom = {0, 0};
    work_coord_from_screen(mouse_screen_coord, &mouse_work_coord_after_zoom);

    work_offset.x +=
        (mouse_work_coord_before_zoom.x - mouse_work_coord_after_zoom.x);
    work_offset.y +=
        (mouse_work_coord_before_zoom.y - mouse_work_coord_after_zoom.y);
}

static void work_state_pan_start(SDL_Event *event, struct app *app)
{
    work_pan_start_point = (SDL_FPoint){event->button.x, event->button.y};
}

static void work_state_pan(SDL_Event *event, struct app *app)
{
    work_offset.x -= (event->motion.x - work_pan_start_point.x) / work_scale;
    work_offset.y -= (event->motion.y - work_pan_start_point.y) / work_scale;
    work_pan_start_point.x = event->motion.x;
    work_pan_start_point.y = event->motion.y;
}

static void work_state_paint(SDL_Event *event, struct app *app)
{
    SDL_FPoint mouse_screen_coord = {event->button.x, event->button.y};
    struct map_tile_layer tile_layer = {app->selected_tileset_index.x,
                                        app->selected_tileset_index.y, 1};
    work_set_tile_layer_on_mouse(mouse_screen_coord, app->map, 0, tile_layer);
}

static void work_state_paint_motion(SDL_Event *event, struct app *app)
{
    SDL_FPoint mouse_screen_coord = {event->motion.x, event->motion.y};
    struct map_tile_layer tile_layer = {app->selected_tileset_index.x,
                                        app->selected_tileset_index.y, 1};
    work_set_tile_layer_on_mouse(mouse_screen_coord, app->map, 0, tile_layer);
}

static void work_state_erase(SDL_Event *event, struct app *app)
{
    SDL_FPoint mouse_screen_coord = {event->button.x, event->button.y};
    struct map_tile_layer tile_layer = {0, 0, 0};
    work_set_tile_layer_on_mouse(mouse_screen_coord, app->map, 0, tile_layer);
}

enum work_state work_get_state(struct app *app, SDL_Event *event)
{
    enum work_state state;

    state = WORK_STATE_ZOOM;
    if (event->type == SDL_MOUSEWHEEL) {
        return state;
    }

    state = WORK_STATE_PAN_START;
    if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_MIDDLE &&
        event->button.clicks == 1) {
        return state;
    }

    state = WORK_STATE_PAN;
    if (event->type == SDL_MOUSEMOTION &&
        event->motion.state == SDL_BUTTON_MMASK) {
        return state;
    }

    state = WORK_STATE_PAINT;
    if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_LEFT &&
        app->work.tool->type == TOOL_TYPE_PENCIL) {
        return state;
    }

    state = WORK_STATE_PAINT_MOTION;
    if (event->type == SDL_MOUSEMOTION &&
        event->motion.state == SDL_BUTTON_LMASK &&
        app->work.tool->type == TOOL_TYPE_PENCIL) {
        return state;
    }

    state = WORK_STATE_ERASE;
    if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_LEFT &&
        app->work.tool->type == TOOL_TYPE_ERASER) {
        return state;
    }

    state = WORK_STATE_IDLE;
    return state;
}

void work_run_state(struct app *app, SDL_Event *event, enum work_state state)
{
    if (state >= WORK_STATE_IDLE)
        return;

    (*work_state_table[state])(event, app);
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
                dst_rect.w = dst_rect.h = tile_size * work_scale;

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
