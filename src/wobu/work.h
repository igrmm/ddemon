#ifndef WORK_H
#define WORK_H

#include "SDL.h" // IWYU pragma: keep //clangd

enum work_state { WORK_STATE_ZOOM = 0, WORK_STATE_IDLE, WORK_STATE_TOTAL };

enum work_tool_type {
    WORK_TOOL_PENCIL = 0,
    WORK_TOOL_ERASER,
    WORK_TOOL_ENTITY,
    WORK_TOOL_SELECT,
    WORK_TOOL_TOTAL
};

struct work_tool {
    enum work_tool_type type;
    SDL_Color rect_color;
    SDL_Texture *icon_texture;
};

struct work_tool_rect {
    SDL_FRect rect;
    SDL_FPoint start;
};

struct work {
    struct work_tool tools[WORK_TOOL_TOTAL];
    struct work_tool_rect tool_rect;
    struct work_tool *tool;
};

struct app;
enum work_state work_get_state(struct app *app, SDL_Event *event);
void work_run_state(struct app *app, SDL_Event *event, enum work_state state);
void work_render(struct app *app);

#endif
