/**
 *  \file core.h
 *
 *  Setup and shutdown SDL library and OpenGL.
 */

#ifndef CORE_H
#define CORE_H

#include <SDL3/SDL.h>

#include "queue.h"

enum core_texture_format {
    CORE_TEXTURE_FORMAT_RGBA = 0,
    CORE_TEXTURE_FORMAT_RED
};

struct core_color {
    float r, g, b, a;
};

struct core_texture {
    float width, height;
    Uint32 id;
};

struct core_drawing {
    // the destination rectangle to be drawn on screen with opengl coords;
    // the texture will be stretched to fill the given rectangle
    float dst_rect_x, dst_rect_y, dst_rect_w, dst_rect_h;
    // the source rectangle representing a portion of the bound texture with
    // opengl coords
    float src_rect_x, src_rect_y, src_rect_w, src_rect_h;
    // texture rgb color
    float r, g, b;
};

struct core_line {
    // line p0
    float x0, y0;
    // line p1
    float x1, y1;
    // line rgb color
    float r, g, b;
};

struct core {
    // SDL
    SDL_Window *window;
    SDL_GLContext ctx;

    // OPENGL
    Uint32 vertex_array_object;
    Uint32 drawing_vertex_buffer_object;
    Uint32 drawing_element_buffer_object;
    Uint32 drawing_instance_vertex_buffer_object;
    Uint32 line_vertex_buffer_object;
    Uint32 line_instance_vertex_buffer_object;
    Uint32 frame_buffer_object;
    Uint32 current_shader;

    // CORE
    int viewport_width, viewport_height;
    struct core_texture current_texture;
    struct core_drawing *drawing_queue;
    struct queue_handle drawing_queue_handle;
    struct core_line *line_queue;
    struct queue_handle line_queue_handle;
};

bool core_setup(struct core *core, const char *window_title, int window_width,
                int window_height, int window_flag);
void core_shutdown(struct core *core);
void core_delete_shader(Uint32 shader);
Uint32 core_create_shader(const char *vert_src, const char *frag_src,
                          int *status, char *log, size_t log_size);
void core_use_shader(struct core *core, Uint32 shader);
void core_delete_texture(struct core_texture *texture);

/**
 * Create an opengl texture from raw pixel array.
 *
 * core_create_texture() create a opengl texture that could be one channel (RED)
 * or four channels (RGBA).
 *
 */
struct core_texture core_create_texture(int width, int height,
                                        enum core_texture_format format,
                                        const Uint8 *pixels);
void core_bind_texture(struct core *core, struct core_texture texture);
void core_clear_screen(float r, float g, float b, float a);

bool core_add_drawing_color_tex(struct core *core, const SDL_FRect *tex_region,
                                const SDL_FRect *src_rect,
                                const SDL_FRect *dst_rect,
                                const struct core_color *color);

/**
 * Add a drawing of a region of a texture to the drawing queue.
 *
 * core_add_drawing_tex() add a region of the bound opengl texture, represented
 * by the rectangle src_rect, to the drawing queue. If the pointer to the rect
 * tex_region is not NULL, src_rect will represent a region of this texture
 * region instead, ant tex_region will represent a portion of the bound texture.
 *
 */
bool core_add_drawing_tex(struct core *core, const SDL_FRect *tex_region,
                          const SDL_FRect *src_rect, const SDL_FRect *dst_rect);
bool core_add_drawing_fill_rect(struct core *core,
                                const SDL_FRect *pixel_tex_region,
                                SDL_FRect *rect, struct core_color *color);
bool core_add_drawing_rect(struct core *core, const SDL_FRect *pixel_tex_region,
                           SDL_FRect *rect, struct core_color *color,
                           float thickness);
bool core_add_line(struct core *core, float x0, float y0, float x1, float y1,
                   struct core_color *color);
void core_render_drawings(struct core *core);
void core_render_lines(struct core *core);
void core_update_window(SDL_Window *window);
void core_update_viewport(struct core *core, int viewport_width,
                          int viewport_height);
void core_offscreen_rendering_begin(struct core *core,
                                    struct core_texture *target_texture);
void core_offscreen_rendering_end(void);

#endif
