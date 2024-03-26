#include "SDL.h" // IWYU pragma: keep //clangd

#include "work.h"

static SDL_FPoint offset;
static SDL_FPoint pan_start;
static float scale;

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
