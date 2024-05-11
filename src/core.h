/**
 *  \file core.h
 *
 *  Setup and shutdown SDL library and OpenGL.
 */

#ifndef CORE_H
#define CORE_H

#include "SDL.h" // IWYU pragma: keep //clangd

#define CORE_DRAWING_POOL_SIZE 13000

struct core_texture {
    float width, height;
    Uint32 id;
};

struct core_drawing {
    // rectangle
    float x, y, w, h;
    // texture rectangle
    float tex_x, tex_y, tex_w, tex_h;
};

struct core {
    // SDL
    SDL_Window *window;
    SDL_GLContext *ctx;

    // OPENGL
    Uint32 vertex_array_object;
    Uint32 vertex_buffer_object;
    Uint32 element_buffer_object;
    Uint32 instance_vertex_buffer_object;
    Uint32 current_shader;

    // CORE
    int viewport_width, viewport_height;
    struct core_texture *current_texture;
    struct core_drawing drawing_pool[CORE_DRAWING_POOL_SIZE];
    int drawing_queue_size;
};

int core_setup(struct core *core, const char *window_title, int window_width,
               int window_height);
void core_shutdown(struct core *core);
void core_delete_shader(Uint32 shader);
Uint32 core_create_shader(const char *vert_src, const char *frag_src,
                          int *status, char *log, size_t log_size);
void core_use_shader(struct core *core, Uint32 shader);
void core_delete_texture(struct core_texture *texture);
struct core_texture core_create_texture(int width, int height,
                                        const Uint8 *texture_data);
void core_bind_texture(struct core *core, struct core_texture *texture);
void core_clear_screen(float r, float g, float b, float a);
void core_add_drawing(struct core *core, SDL_FRect *src_rect,
                      SDL_FRect *dst_rect);
void core_draw_queue(struct core *core);
void core_update_window(SDL_Window *window);
void core_update_viewport(struct core *core, int viewport_width,
                          int viewport_height);

#endif
