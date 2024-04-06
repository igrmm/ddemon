#include "SDL.h" // IWYU pragma: keep //clangd
#include "SDL_image.h"
#include "nk.h"

#include "../core.h"
#include "../ecs.h"
#include "app.h"
#include "menu.h"
#include "pick.h"
#include "properties.h"
#include "status.h"
#include "tools.h"
#include "work.h"

int app_init(struct app *app, struct core *core, struct nk_context *nk_ctx)
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

    SDL_Texture *tileset_texture =
        IMG_LoadTexture(core->renderer, "tileset.png");
    if (tileset_texture == NULL) {
        SDL_Log("SDL error in tileset_texture creation: %s", SDL_GetError());
        return -1;
    }

    SDL_Texture *pencil_icon_texture =
        IMG_LoadTexture(core->renderer, "../../assets/pencil.png");
    if (pencil_icon_texture == NULL) {
        SDL_Log("SDL error in pencil_texture creation: %s", SDL_GetError());
        return -1;
    }
    app->work.tools[TOOL_TYPE_PENCIL] = (struct tool){
        TOOL_TYPE_PENCIL, (SDL_Color){0, 255, 0, 255}, pencil_icon_texture};

    SDL_Texture *eraser_icon_texture =
        IMG_LoadTexture(core->renderer, "../../assets/eraser.png");
    if (eraser_icon_texture == NULL) {
        SDL_Log("SDL error in eraser_texture creation: %s", SDL_GetError());
        return -1;
    }
    app->work.tools[TOOL_TYPE_ERASER] = (struct tool){
        TOOL_TYPE_ERASER, (SDL_Color){255, 0, 0, 255}, eraser_icon_texture};

    SDL_Texture *entity_icon_texture =
        IMG_LoadTexture(core->renderer, "../../assets/entity.png");
    if (entity_icon_texture == NULL) {
        SDL_Log("SDL error in entity_texture creation: %s", SDL_GetError());
        return -1;
    }
    app->work.tools[TOOL_TYPE_ENTITY] = (struct tool){
        TOOL_TYPE_ENTITY, (SDL_Color){255, 0, 0, 255}, entity_icon_texture};

    SDL_Texture *select_icon_texture =
        IMG_LoadTexture(core->renderer, "../../assets/select.png");
    if (select_icon_texture == NULL) {
        SDL_Log("SDL error in select_texture creation: %s", SDL_GetError());
        return -1;
    }
    app->work.tools[TOOL_TYPE_SELECT] = (struct tool){
        TOOL_TYPE_SELECT, (SDL_Color){255, 0, 0, 255}, select_icon_texture};

    app->work.tool = &app->work.tools[TOOL_TYPE_PENCIL];
    app->tileset_texture = tileset_texture;
    app->show_pick_window = 1;
    app->show_grid = 1;
    app->show_tool_window = 1;
    app->show_properties_window = 1;
    app->window_flags = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                        NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                        NK_WINDOW_CLOSABLE;
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

    if (app->show_properties_window)
        properties_window(app);

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

    if (app->tileset_texture != NULL)
        SDL_DestroyTexture(app->tileset_texture);

    enum tool_type tool_type = 0;
    for (; tool_type < TOOL_TYPE_TOTAL; tool_type++) {
        if (app->work.tools[tool_type].icon_texture != NULL)
            SDL_DestroyTexture(app->work.tools[tool_type].icon_texture);
    }
}
