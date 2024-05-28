#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#include "../core.h"
#include "../ecs.h"
#include "app.h"
#include "menu.h"
#include "pick.h"
#include "prop.h"
#include "status.h"
#include "tools.h"
#include "work.h"

int app_init(struct app *app, struct core *core, struct assets *assets,
             struct nk_context *nk_ctx)
{
    app->map = SDL_calloc(1, sizeof(*app->map));
    if (app->map == NULL)
        return -1;

    app->ecs = ecs_create();
    if (app->ecs == NULL)
        return -1;

    app->selected_entities = ecs_create_table();
    if (app->selected_entities == NULL)
        return -1;

    app->core = core;
    app->nk_ctx = nk_ctx;

    struct core_texture tileset_texture =
        assets->textures[ASSET_TEXTURE_TILEMAP];

    struct core_texture pencil_icon_texture =
        assets->textures[ASSET_TEXTURE_ICON_PENCIL];
    app->work.tools[TOOL_TYPE_PENCIL] = (struct tool){
        TOOL_TYPE_PENCIL, (struct core_color){0.0f, 1.0f, 0.0f, 1.0f},
        pencil_icon_texture};

    struct core_texture eraser_icon_texture =
        assets->textures[ASSET_TEXTURE_ICON_ERASER];
    app->work.tools[TOOL_TYPE_ERASER] = (struct tool){
        TOOL_TYPE_ERASER, (struct core_color){1.0f, 0.0f, 0.0f, 1.0f},
        eraser_icon_texture};

    struct core_texture entity_icon_texture =
        assets->textures[ASSET_TEXTURE_ICON_ENTITY];
    app->work.tools[TOOL_TYPE_ENTITY] = (struct tool){
        TOOL_TYPE_ENTITY, (struct core_color){1.0f, 0.0f, 0.0f, 1.0f},
        entity_icon_texture};

    struct core_texture select_icon_texture =
        assets->textures[ASSET_TEXTURE_ICON_SELECT];
    app->work.tools[TOOL_TYPE_SELECT] = (struct tool){
        TOOL_TYPE_SELECT, (struct core_color){1.0f, 0.0f, 0.0f, 1.0f},
        select_icon_texture};

    app->work.tool = &app->work.tools[TOOL_TYPE_PENCIL];
    app->tileset_texture = tileset_texture;
    app->show_pick_window = 1;
    app->show_grid = 1;
    app->show_tool_window = 1;
    app->show_prop_window = 1;
    app->window_flags = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                        NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                        NK_WINDOW_CLOSABLE;
    app->default_shader = assets->shaders[ASSET_SHADER_DEFAULT];
    app->primitive_shader = assets->shaders[ASSET_SHADER_PRIMITIVE];

    return 0;
}

void app_handle_event(struct app *app, SDL_Event *event)
{
    enum work_state state = work_get_state(app, event);
    work_run_state(app, event, state);
}

void app_run(struct app *app)
{
    if (app->show_pick_window)
        pick_window(app);

    if (app->show_tool_window)
        tools_window(app);

    if (app->show_prop_window)
        prop_window(app);

    menu_window(app);
    status_window(app);
}

void app_render(struct app *app) { work_render(app); }

void app_shutdown(struct app *app)
{
    if (app->map != NULL)
        SDL_free(app->map);

    if (app->ecs != NULL)
        SDL_free(app->ecs);

    if (app->selected_entities != NULL)
        SDL_free(app->selected_entities);
}
