/**
 *  \file core.h
 *
 *  Setup and shutdown SDL library and OpenGL.
 */

#ifndef CORE_H
#define CORE_H

#include "SDL.h" // IWYU pragma: keep //clangd

#define CORE_DRAWING_POOL_SIZE 13000

struct core_color {
    float r, g, b, a;
};

struct core_texture {
    float width, height;
    Uint32 id;
};

struct core_drawing {
    // rectangle
    float x, y, w, h;
    // texture rectangle or rgba color
    float data1, data2, data3, data4;
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
    Uint32 frame_buffer_object;
    Uint32 current_shader;

    // CORE
    int viewport_width, viewport_height;
    struct core_texture current_texture;
    struct core_drawing drawing_pool[CORE_DRAWING_POOL_SIZE];
    int drawing_queue_size;
};

int core_setup(struct core *core, const char *window_title, int window_width,
               int window_height, int window_flag);
void core_shutdown(struct core *core);
void core_delete_shader(Uint32 shader);
Uint32 core_create_shader(const char *vert_src, const char *frag_src,
                          int *status, char *log, size_t log_size);
void core_use_shader(struct core *core, Uint32 shader);
void core_delete_texture(struct core_texture *texture);

/**
 * Create an opengl texture from a image loaded with the stbi library.
 *
 * core_create_stbi_texture() uses GL_UNPACK_ALIGNMENT of value 4 (the default
 * value) so it is expected that the width of the image is power of two and a
 * 4 channel image (RGBA)
 *
 */
struct core_texture core_create_stbi_texture(int width, int height,
                                             const Uint8 *texture_data);
/**
 * Create an opengl texture from a image loaded with the stbtt library.
 *
 * core_create_stbtt_texture() uses GL_UNPACK_ALIGNMENT of value 1 because
 * stb_truetype may create bitmap with width not power of two and only one
 * channel(R->ALPHA)
 *
 */
struct core_texture core_create_stbtt_texture(int width, int height,
                                              const Uint8 *texture_data);
void core_bind_texture(struct core *core, struct core_texture texture);
void core_clear_screen(float r, float g, float b, float a);

/**
 * Add a drawing of a region of a texture to the drawing queue.
 *
 * core_add_drawing_tex() add a region of the bound opengl texture, represented
 * by the rectangle src_rect, to the drawing queue. If the pointer to the rect
 * tex_region is not NULL, src_rect will represent a region of this texture
 * region instead, ant tex_region will represent a portion of the bound texture.
 *
 */
void core_add_drawing_tex(struct core *core, const SDL_FRect *tex_region,
                          const SDL_FRect *src_rect, const SDL_FRect *dst_rect);
void core_add_drawing_fill_rect(struct core *core, SDL_FRect *rect,
                                struct core_color *color);
void core_add_drawing_rect(struct core *core, SDL_FRect *rect,
                           struct core_color *color, float thickness);
void core_draw_queue(struct core *core);
void core_update_window(SDL_Window *window);
void core_update_viewport(struct core *core, int viewport_width,
                          int viewport_height);
void core_restore_gl_state(struct core *core);
void core_offscreen_rendering_begin(struct core *core,
                                    struct core_texture *target_texture);
void core_offscreen_rendering_end(void);

#endif
