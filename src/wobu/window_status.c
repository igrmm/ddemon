#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#include "../core.h"
#include "app.h"
#include "windows.h"

static int frames;
static Uint32 timer;
static char fps_text[9];

void window_status(struct app *app)
{
    // get fps
    Uint32 now = SDL_GetTicks64();
    if (now - timer >= 1000) {
        int fps_number = SDL_min(frames, 999);
        SDL_snprintf(fps_text, sizeof(fps_text), "fps: %i", fps_number);
        timer = now;
        frames = 0;
    }
    frames++;

    struct nk_context *nk_ctx = app->nk_ctx;
    int h = 20;
    int y = app->core->window_height - h;
    int x = 0;
    int w = app->core->window_width;

    if (nk_begin(nk_ctx, "status", nk_rect(x, y, w, h),
                 NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(nk_ctx, h, 1);
        nk_label(nk_ctx, fps_text, NK_TEXT_ALIGN_LEFT);
    }
    nk_end(nk_ctx);
}
