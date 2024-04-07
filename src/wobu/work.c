#include "SDL.h" // IWYU pragma: keep //clangd

#include "../core.h"
#include "../ecs.h"
#include "app.h"
#include "map.h"
#include "work.h"

static SDL_FPoint work_offset;
static SDL_FPoint work_pan_start_point;
static float work_scale = 1;

static void work_state_zoom(SDL_Event *event, struct app *app);
static void work_state_pan_start(SDL_Event *event, struct app *app);
static void work_state_pan(SDL_Event *event, struct app *app);
static void work_state_mk_tool_rect(SDL_Event *event, struct app *app);
static void work_state_paint(SDL_Event *event, struct app *app);
static void work_state_paint_motion(SDL_Event *event, struct app *app);
static void work_state_paint_rect(SDL_Event *event, struct app *app);
static void work_state_erase(SDL_Event *event, struct app *app);
static void work_state_erase_motion(SDL_Event *event, struct app *app);
static void work_state_erase_rect(SDL_Event *event, struct app *app);
static void work_state_entity_rect(SDL_Event *event, struct app *app);

static void (*work_state_table[WORK_STATE_TOTAL])(SDL_Event *event,
                                                  struct app *app) = {
    // clang-format off
    work_state_zoom,
    work_state_pan_start,
    work_state_pan,
    work_state_mk_tool_rect,
    work_state_paint,
    work_state_paint_motion,
    work_state_paint_rect,
    work_state_erase,
    work_state_erase_motion,
    work_state_erase_rect,
    work_state_entity_rect,
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

static void work_mk_tool_rect(struct tool_rect *tool_rect,
                              SDL_FPoint mouse_screen_coord)
{
    // HORIZONTAL X AXIS
    if (tool_rect->start.x <= 0) {
        tool_rect->start.x = tool_rect->rect.x = mouse_screen_coord.x;
    } else if (mouse_screen_coord.x > tool_rect->start.x) {
        tool_rect->rect.x = tool_rect->start.x;
        tool_rect->rect.w = mouse_screen_coord.x - tool_rect->start.x;
    } else {
        tool_rect->rect.w = tool_rect->start.x - mouse_screen_coord.x;
        tool_rect->rect.x = mouse_screen_coord.x;
    }

    // VERTICAL Y AXIS
    if (tool_rect->start.y <= 0) {
        tool_rect->start.y = tool_rect->rect.y = mouse_screen_coord.y;
    } else if (mouse_screen_coord.y > tool_rect->start.y) {
        tool_rect->rect.y = tool_rect->start.y;
        tool_rect->rect.h = mouse_screen_coord.y - tool_rect->start.y;
    } else {
        tool_rect->rect.h = tool_rect->start.y - mouse_screen_coord.y;
        tool_rect->rect.y = mouse_screen_coord.y;
    }
}

static void work_mk_tile_shaped_tool_rect(struct tool_rect *tool_rect,
                                          SDL_FPoint mouse_screen_coord)
{
    work_mk_tool_rect(tool_rect, mouse_screen_coord);

    // make tool_rect in work coords
    SDL_FRect tool_rect_work_coord = {0};
    work_rect_from_screen(tool_rect->rect, &tool_rect_work_coord);

    // make map_rect in work coords
    int map_width = MAP_TILES_X_MAX * MAP_TILE_SIZE;
    int map_height = MAP_TILES_Y_MAX * MAP_TILE_SIZE;
    SDL_FRect map_rect_work_coord = {0, 0, map_width, map_height};

    SDL_FRect intersect = {0};
    if (SDL_IntersectFRect(&map_rect_work_coord, &tool_rect_work_coord,
                           &intersect)) {
        int tile_size = MAP_TILE_SIZE;

        // floor intersect origin to tile index (i*tile) and save the difference
        int x = (int)(intersect.x / tile_size) * tile_size;
        int diff_x = intersect.x - x;
        intersect.x = x;
        int y = (int)(intersect.y / tile_size) * tile_size;
        int diff_y = intersect.y - y;
        intersect.y = y;

        // ceil intersect sz to tile index (j*tile) n clamp if bigger than mapsz
        intersect.w = (int)((intersect.w + diff_x) / tile_size + 1) * tile_size;
        if ((intersect.x + intersect.w) > map_rect_work_coord.w)
            intersect.w = map_rect_work_coord.w - intersect.x;
        intersect.h = (int)((intersect.h + diff_y) / tile_size + 1) * tile_size;
        if ((intersect.y + intersect.h) > map_rect_work_coord.h)
            intersect.h = map_rect_work_coord.h - intersect.y;

        tool_rect->rect = intersect;
    } else {
        tool_rect->rect = (SDL_FRect){0};
    }
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

static void work_set_tile_layers_in_rect(SDL_FRect rect_work_coord,
                                         struct map *map, int layer,
                                         struct map_tile_layer tile_layer)
{
    if (!SDL_FRectEmpty(&rect_work_coord)) {
        int i0 = rect_work_coord.x / MAP_TILE_SIZE;
        int j0 = rect_work_coord.y / MAP_TILE_SIZE;
        int i1 = (rect_work_coord.x + rect_work_coord.w) / MAP_TILE_SIZE;
        int j1 = (rect_work_coord.y + rect_work_coord.h) / MAP_TILE_SIZE;
        for (int i = i0; i < i1; i++) {
            for (int j = j0; j < j1; j++) {
                map->tiles[i][j].layers[layer] = tile_layer;
            }
        }
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

static void work_state_mk_tool_rect(SDL_Event *event, struct app *app)
{
    work_mk_tile_shaped_tool_rect(
        &app->work.tool_rect, (SDL_FPoint){event->motion.x, event->motion.y});
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

static void work_state_paint_rect(SDL_Event *event, struct app *app)
{
    struct map_tile_layer tile_layer = {app->selected_tileset_index.x,
                                        app->selected_tileset_index.y, 1};
    work_set_tile_layers_in_rect(app->work.tool_rect.rect, app->map, 0,
                                 tile_layer);
    app->work.tool_rect.rect = (SDL_FRect){0};
    app->work.tool_rect.start = (SDL_FPoint){0};
}

static void work_state_erase(SDL_Event *event, struct app *app)
{
    SDL_FPoint mouse_screen_coord = {event->button.x, event->button.y};
    struct map_tile_layer tile_layer = {0, 0, 0};
    work_set_tile_layer_on_mouse(mouse_screen_coord, app->map, 0, tile_layer);
}

static void work_state_erase_motion(SDL_Event *event, struct app *app)
{
    SDL_FPoint mouse_screen_coord = {event->motion.x, event->motion.y};
    struct map_tile_layer tile_layer = {0, 0, 0};
    work_set_tile_layer_on_mouse(mouse_screen_coord, app->map, 0, tile_layer);
}

static void work_state_erase_rect(SDL_Event *event, struct app *app)
{
    struct map_tile_layer tile_layer = {0, 0, 0};
    work_set_tile_layers_in_rect(app->work.tool_rect.rect, app->map, 0,
                                 tile_layer);
    app->work.tool_rect.rect = (SDL_FRect){0};
    app->work.tool_rect.start = (SDL_FPoint){0};
}

static void work_state_entity_rect(SDL_Event *event, struct app *app)
{
    // clear selected_entities
    Uint16 entity;
    struct ecs_node *node = NULL;
    while (ecs_table_iterate_entities(app->selected_entities, &node, &entity))
        ecs_table_remove_entity(app->selected_entities, entity);

    enum component_tag_type tag = CMP_TAG_WAYPOINT;
    entity = ecs_create_entity(app->ecs);
    struct component cmp_tag = {.type = CMP_TYPE_TAG,
                                .entity = entity,
                                .alive = SDL_TRUE,
                                .data = {.tag = {tag}}};
    ecs_add_component(app->ecs, cmp_tag);
    struct component cmp_rect = {.type = CMP_TYPE_RECT,
                                 .entity = entity,
                                 .alive = SDL_TRUE,
                                 .data = {.rect = {app->work.tool_rect.rect}}};
    ecs_add_component(app->ecs, cmp_rect);
    ecs_table_add_entity(app->selected_entities, entity);
    app->work.tool_rect.rect = (SDL_FRect){0};
    app->work.tool_rect.start = (SDL_FPoint){0};
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

    state = WORK_STATE_MK_TOOL_RECT;
    if (event->type == SDL_MOUSEMOTION &&
        event->motion.state == SDL_BUTTON_RMASK) {
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

    state = WORK_STATE_PAINT_RECT;
    if (event->type == SDL_MOUSEBUTTONUP &&
        event->button.button == SDL_BUTTON_RIGHT &&
        app->work.tool->type == TOOL_TYPE_PENCIL) {
        return state;
    }

    state = WORK_STATE_ERASE;
    if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_LEFT &&
        app->work.tool->type == TOOL_TYPE_ERASER) {
        return state;
    }

    state = WORK_STATE_ERASE_MOTION;
    if (event->type == SDL_MOUSEMOTION &&
        event->motion.state == SDL_BUTTON_LMASK &&
        app->work.tool->type == TOOL_TYPE_ERASER) {
        return state;
    }

    state = WORK_STATE_ERASE_RECT;
    if (event->type == SDL_MOUSEBUTTONUP &&
        event->button.button == SDL_BUTTON_RIGHT &&
        app->work.tool->type == TOOL_TYPE_ERASER) {
        return state;
    }

    state = WORK_STATE_ENTITY_RECT;
    if (event->type == SDL_MOUSEBUTTONUP &&
        event->button.button == SDL_BUTTON_RIGHT &&
        app->work.tool->type == TOOL_TYPE_ENTITY) {
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

    // render entities
    struct ecs_node *node = NULL;
    Uint16 entity;
    while (ecs_iterate_entities(app->ecs, &node, &entity)) {
        struct component *cmp_rect =
            ecs_get_component(app->ecs, CMP_TYPE_RECT, entity);
        SDL_FRect rect_work_coord = cmp_rect->data.rect.rect;
        SDL_FRect rect_screen_coord;
        work_rect_to_screen(rect_work_coord, &rect_screen_coord);
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderDrawRectF(renderer, &rect_screen_coord);
    }

    // render selected entities
    node = NULL;
    while (ecs_table_iterate_entities(app->selected_entities, &node, &entity)) {
        struct component *cmp_rect =
            ecs_get_component(app->ecs, CMP_TYPE_RECT, entity);
        SDL_FRect rect_work_coord = cmp_rect->data.rect.rect;
        SDL_FRect rect_screen_coord;
        work_rect_to_screen(rect_work_coord, &rect_screen_coord);
        SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, 70);
        SDL_RenderFillRectF(renderer, &rect_screen_coord);
    }

    // render tool rect
    if (app->work.tool_rect.rect.w > 0 || app->work.tool_rect.rect.h > 0) {
        SDL_Color color = app->work.tool->rect_color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_FRect tool_rect_screen_coord = {0};
        work_rect_to_screen(app->work.tool_rect.rect, &tool_rect_screen_coord);
        SDL_RenderDrawRectF(renderer, &tool_rect_screen_coord);
    }
}
