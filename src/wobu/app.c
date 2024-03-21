#include "app.h"
#include "windows.h"

int app_init(struct app *app, struct core *core, struct nk_context *nk_ctx)
{
    app->core = core;
    app->nk_ctx = nk_ctx;
    return 0;
}

void app_run(struct app *app) { window_status(app); }
