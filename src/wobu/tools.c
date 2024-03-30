#include "nk.h"

#include "tools.h"
#include "app.h"
#include "work.h"

void tools_window(struct app *app)
{
    struct nk_context *nk_ctx = app->nk_ctx;
    int window_flags = app->window_flags;

    if (nk_begin(nk_ctx, "tools", nk_rect(20, 260, 200, 85), window_flags)) {
        int btn_size = 36;
        struct nk_color color_bkp = nk_ctx->style.button.border_color;

        nk_layout_row_static(nk_ctx, btn_size, btn_size, 4);
        enum tool_type tool_type = 0;
        for (; tool_type < TOOL_TYPE_TOTAL; tool_type++) {
            if (app->work.tool->type == tool_type)
                nk_ctx->style.button.border_color = RED;
            if (nk_button_image(
                    nk_ctx,
                    nk_image_ptr(app->work.tools[tool_type].icon_texture)))
                app->work.tool = &app->work.tools[tool_type];
            nk_ctx->style.button.border_color = color_bkp;
        }
    } else {
        app->show_tool_window = 0;
    }
    nk_end(nk_ctx);
}
