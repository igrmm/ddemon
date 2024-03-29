#ifndef WORK_H
#define WORK_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "tools.h"

enum work_state {
    WORK_STATE_ZOOM = 0,
    WORK_STATE_PAN_START,
    WORK_STATE_PAN,
    WORK_STATE_MK_TOOL_RECT,
    WORK_STATE_PAINT,
    WORK_STATE_PAINT_MOTION,
    WORK_STATE_ERASE,
    WORK_STATE_ERASE_MOTION,
    WORK_STATE_IDLE,
    WORK_STATE_TOTAL
};

struct work {
    struct tool tools[TOOL_TYPE_TOTAL];
    struct tool_rect tool_rect;
    struct tool *tool;
};

struct app;
enum work_state work_get_state(struct app *app, SDL_Event *event);
void work_run_state(struct app *app, SDL_Event *event, enum work_state state);
void work_render(struct app *app);

#endif
