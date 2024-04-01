#include "properties.h"
#include "app.h"

void properties_window(struct app *app)
{
    struct nk_context *nk_ctx = app->nk_ctx;
    int window_flags = app->window_flags;

    if (nk_begin(nk_ctx, "properties", nk_rect(20, 365, 200, 200),
                 window_flags)) {
        nk_layout_row_dynamic(nk_ctx, 25, 1);
        nk_label(nk_ctx, "no selection", NK_TEXT_LEFT);
    } else {
        app->show_properties_window = 0;
    }
    nk_end(nk_ctx);
}
