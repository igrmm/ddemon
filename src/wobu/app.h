#ifndef APP_H
#define APP_H

struct app {
    struct core *core;
    struct nk_context *nk_ctx;
};

int app_init(struct app *app, struct core *core, struct nk_context *nk_ctx);
void app_run(struct app *app);

#endif
