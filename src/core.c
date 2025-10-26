#define GLAD_GL_IMPLEMENTATION
#include "../external/glad.h"

#include <SDL3/SDL.h>

#include "core.h"
#include "queue.h"

#define CORE_DRAWING_QUEUE_CAPACITY 13000
#define CORE_LINE_QUEUE_CAPACITY 500000

bool core_setup(struct core *core, const char *window_title, int window_width,
                int window_height, int window_flag)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

#if defined(SDL_PLATFORM_ANDROID)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    core->window = SDL_CreateWindow(window_title, window_width, window_height,
                                    window_flag | SDL_WINDOW_OPENGL);
    if (core->window == NULL) {
        SDL_Log("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return false;
    }

    core->ctx = SDL_GL_CreateContext(core->window);
    core->viewport_width = window_width;
    core->viewport_height = window_height;

    // try to init glad with ogl profile
    const char *platform = SDL_GetPlatform();
    int profile, version;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
    if (profile == SDL_GL_CONTEXT_PROFILE_ES) {
        version = gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress);
    } else {
        version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    }
    if (!version) {
        SDL_Log("Failed to initialize GLAD (OGL/OGLES) on platform: %s",
                platform);
        return false;
    }
    SDL_Log("GLAD initialization succeeded.");
    SDL_Log("PLATFORM: %s, PROFILE: %i, VERSION: %i", platform, profile,
            version);

    // malloc memory for drawing queue
    core->drawing_queue =
        SDL_malloc(CORE_DRAWING_QUEUE_CAPACITY * sizeof(struct core_drawing));
    if (core->drawing_queue == NULL) {
        SDL_Log("Error in core_setup(): malloc failed (drawing_queue)");
        return false;
    }
    queue_initialize(&core->drawing_queue_handle, CORE_DRAWING_QUEUE_CAPACITY);

    // malloc memory for line queue
    core->line_queue =
        SDL_malloc(CORE_LINE_QUEUE_CAPACITY * sizeof(struct core_line));
    if (core->line_queue == NULL) {
        SDL_Log("Error in core_setup(): malloc failed (line_queue)");
        return false;
    }
    queue_initialize(&core->line_queue_handle, CORE_LINE_QUEUE_CAPACITY);

    // vsync off so we can see fps
    SDL_GL_SetSwapInterval(0);

    // enable transparency
    glEnable(GL_BLEND);
    // this step is necessary to preserve alpha channel because we create atlas
    // texture with offscreen rendering, see atlas shader and
    // https://stackoverflow.com/questions/24346585/opengl-render-to-texture-with-partial-transparancy-translucency-and-then-rende
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA,
                        GL_ONE);

    // create fbo, will be used for postprocessing, offscreen rendering etc
    glGenFramebuffers(1, &core->frame_buffer_object);

    // create vao
    glGenVertexArrays(1, &core->vertex_array_object);
    glBindVertexArray(core->vertex_array_object);

    // setup instance vbo for the drawings
    glGenBuffers(1, &core->drawing_instance_vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->drawing_instance_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(struct core_drawing) * CORE_DRAWING_QUEUE_CAPACITY,
                 NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind drawing_instance_vbo

    // setup vertices/indices of quad for the template drawing
    float drawing_vertices[] = {
        // w     h     <- identify if this is a "width" or "height" vertex
        1.0f, 1.0f, // top right
        1.0f, 0.0f, // bottom right
        0.0f, 0.0f, // bottom left
        0.0f, 1.0f, // top left
    };
    Uint32 drawing_indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // setup vbo for the quad template
    glGenBuffers(1, &core->drawing_vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->drawing_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(drawing_vertices), drawing_vertices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo

    // setup ebo for the quad template
    glGenBuffers(1, &core->drawing_element_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, core->drawing_element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(drawing_indices),
                 drawing_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

    // put the vertex atrib of the quad template vbo in the vao
    glBindBuffer(GL_ARRAY_BUFFER, core->drawing_vertex_buffer_object);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo

    // put the vertex atrib of the instance vbo in the vao
    glBindBuffer(GL_ARRAY_BUFFER, core->drawing_instance_vertex_buffer_object);
    glEnableVertexAttribArray(1);
    // the first 4 floats of struct core_drawing (dst rect x,y,w,h)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (void *)0);
    glVertexAttribDivisor(1, 1); // create instance
    glEnableVertexAttribArray(2);
    // the next 4 floats of struct core_drawing (src rect x,y,w,h)
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (void *)(4 * sizeof(float)));
    glVertexAttribDivisor(2, 1); // create instance
    glEnableVertexAttribArray(3);
    // the last 3 floats of struct core_drawing (rgb color)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (void *)(8 * sizeof(float)));
    glVertexAttribDivisor(3, 1);      // create instance
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind instance vbo

    // setup instance vbo for the lines
    glGenBuffers(1, &core->line_instance_vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->line_instance_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(struct core_line) * CORE_LINE_QUEUE_CAPACITY, NULL,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind line_instance_vbo

    // setup vertices of line for the template drawing
    float line_vertices[] = {
        // x     y
        0.0f, 0.0f, // p0
        1.0f, 1.0f  // p1
    };

    // setup vbo for the line template
    glGenBuffers(1, &core->line_vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->line_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo

    // put the vertex atrib of the line template vbo in the vao
    glBindBuffer(GL_ARRAY_BUFFER, core->line_vertex_buffer_object);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo

    // put the vertex atrib of the line instance vbo in the vao
    glBindBuffer(GL_ARRAY_BUFFER, core->line_instance_vertex_buffer_object);
    glEnableVertexAttribArray(5);
    // the first 2 floats of struct core_line (x0, y0)
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float),
                          (void *)0);
    glVertexAttribDivisor(5, 1); // create instance
    glEnableVertexAttribArray(6);
    // the next 2 floats of struct core_line (x1, y1)
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glVertexAttribDivisor(6, 1); // create instance
    glEnableVertexAttribArray(7);
    // the last 3 floats of struct core_line (rgb color)
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float),
                          (void *)(4 * sizeof(float)));
    glVertexAttribDivisor(7, 1);      // create instance
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind instance vbo

    // bind drawing ebo to be used throughout the whole program. We dont use ebo
    // for the line rendering
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, core->drawing_element_buffer_object);

    return true;
}

void core_shutdown(struct core *core)
{
    if (core->frame_buffer_object > 0)
        glDeleteFramebuffers(1, &core->frame_buffer_object);

    if (core->vertex_array_object > 0)
        glDeleteVertexArrays(1, &core->vertex_array_object);

    if (core->drawing_instance_vertex_buffer_object > 0)
        glDeleteBuffers(1, &core->drawing_instance_vertex_buffer_object);

    if (core->drawing_vertex_buffer_object > 0)
        glDeleteBuffers(1, &core->drawing_vertex_buffer_object);

    if (core->drawing_element_buffer_object > 0)
        glDeleteBuffers(1, &core->drawing_element_buffer_object);

    if (core->line_instance_vertex_buffer_object > 0)
        glDeleteBuffers(1, &core->line_instance_vertex_buffer_object);

    if (core->line_vertex_buffer_object > 0)
        glDeleteBuffers(1, &core->line_vertex_buffer_object);

    SDL_free(core->drawing_queue);
    SDL_free(core->line_queue);
    SDL_GL_DestroyContext(core->ctx);
    SDL_DestroyWindow(core->window);
    SDL_Quit();
}

void core_delete_shader(Uint32 shader) { glDeleteProgram(shader); }

Uint32 core_create_shader(const char *vert_src, const char *frag_src,
                          int *status, char *log, size_t log_size)
{
    //  vertex shader
    Uint32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vert_src, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, status);
    if (!*status)
        glGetShaderInfoLog(vertex_shader, log_size, NULL, log);

    // fragment shader
    Uint32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_src, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, status);
    if (!*status)
        glGetShaderInfoLog(fragment_shader, log_size, NULL, log);

    // shader program
    Uint32 shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, status);
    if (!*status)
        glGetProgramInfoLog(shader_program, log_size, NULL, log);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

void core_use_shader(struct core *core, Uint32 shader)
{
    if (core->current_shader != shader) {
        glUseProgram(shader);
        core->current_shader = shader;
    }
}

void core_delete_texture(struct core_texture *texture)
{
    glDeleteTextures(1, &texture->id);
}

struct core_texture core_create_texture(int width, int height,
                                        enum core_texture_format format,
                                        const Uint8 *pixels)
{
    // assume that non 1 channel images will be rgba images
    Sint32 opengl_format = GL_RGBA;
    if (format == CORE_TEXTURE_FORMAT_RED) {
        opengl_format = GL_RED;
    }

    // unpack alignment of 1 because some images could be not power of two
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    Uint32 texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // this step necessary to make 1 channel red image into white one w/ alpha
    if (format == CORE_TEXTURE_FORMAT_RED) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, opengl_format);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, opengl_format, width, height, 0,
                 opengl_format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind texture

    struct core_texture texture = {width, height, texture_id};

    return texture;
}

void core_bind_texture(struct core *core, struct core_texture texture)
{
    if (core->current_texture.id != texture.id) {
        glBindTexture(GL_TEXTURE_2D, texture.id);
        core->current_texture = texture;
    }
}

void core_clear_screen(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

bool core_add_drawing_color_tex(struct core *core, const SDL_FRect *tex_region,
                                const SDL_FRect *src_rect,
                                const SDL_FRect *dst_rect,
                                const struct core_color *color)
{
    int instance;
    if (!queue_add(&instance, &core->drawing_queue_handle))
        return false;

    float offset_x = 0, offset_y = 0;

    // verify if drawing is part of other texture
    if (tex_region != NULL) {
        offset_x = tex_region->x;
        offset_y = tex_region->y;
    }

    // check given color
    struct core_color c = {1.0f, 1.0f, 1.0f, 0.0f};
    if (color != NULL)
        c = *color;

    // set up instance to be draw with opengl coords
    core->drawing_queue[instance] = (struct core_drawing){
        .dst_rect_x = dst_rect->x / core->viewport_width * 2.0f - 1.0f,
        .dst_rect_y = dst_rect->y / core->viewport_height * 2.0f - 1.0f,
        .dst_rect_w = 1.0f / core->viewport_width * 2.0f * dst_rect->w,
        .dst_rect_h = 1.0f / core->viewport_height * 2.0f * dst_rect->h,
        .src_rect_x = (src_rect->x + offset_x) / core->current_texture.width,
        .src_rect_y = (src_rect->y + offset_y) / core->current_texture.height,
        .src_rect_w = src_rect->w / core->current_texture.width,
        .src_rect_h = src_rect->h / core->current_texture.height,
        .r = c.r,
        .g = c.g,
        .b = c.b};

    return true;
}

bool core_add_drawing_tex(struct core *core, const SDL_FRect *tex_region,
                          const SDL_FRect *src_rect, const SDL_FRect *dst_rect)
{
    return core_add_drawing_color_tex(core, tex_region, src_rect, dst_rect,
                                      NULL);
}

bool core_add_drawing_fill_rect(struct core *core,
                                const SDL_FRect *pixel_tex_region,
                                SDL_FRect *rect, struct core_color *color)
{
    SDL_FRect src_rect = {0, 0, pixel_tex_region->w, pixel_tex_region->h};
    return core_add_drawing_color_tex(core, pixel_tex_region, &src_rect, rect,
                                      color);
}

bool core_add_drawing_rect(struct core *core, const SDL_FRect *pixel_tex_region,
                           SDL_FRect *rect, struct core_color *color,
                           float thickness)
{
    SDL_FRect line = {0};

    // line bottom
    line.x = rect->x;
    line.y = rect->y;
    line.w = rect->w;
    line.h = thickness;
    if (!core_add_drawing_fill_rect(core, pixel_tex_region, &line, color))
        return false;

    // line top
    line.y = rect->y + rect->h - thickness;
    if (!core_add_drawing_fill_rect(core, pixel_tex_region, &line, color))
        return false;

    // line left
    line.y = rect->y;
    line.w = thickness;
    line.h = rect->h;
    if (!core_add_drawing_fill_rect(core, pixel_tex_region, &line, color))
        return false;

    // line right
    line.x = rect->x + rect->w - thickness;
    if (!core_add_drawing_fill_rect(core, pixel_tex_region, &line, color))
        return false;

    return true;
}

bool core_add_line(struct core *core, float x0, float y0, float x1, float y1,
                   struct core_color *color)
{
    int instance;
    if (!queue_add(&instance, &core->line_queue_handle)) {
        return false;
    }

    struct core_color c = {0.0f, 0.0f, 0.0f, 0.0f};
    if (color != NULL)
        c = *color;

    core->line_queue[instance] =
        (struct core_line){.x0 = x0 / core->viewport_width * 2.0f - 1.0f,
                           .y0 = y0 / core->viewport_height * 2.0f - 1.0f,
                           .x1 = (x1 - x0) / core->viewport_width * 2.0f,
                           .y1 = (y1 - y0) / core->viewport_height * 2.0f,
                           .r = c.r,
                           .g = c.g,
                           .b = c.b};

    return true;
}

void core_render_drawings(struct core *core)
{
    glBindBuffer(GL_ARRAY_BUFFER, core->drawing_instance_vertex_buffer_object);
    int drawing_count = core->drawing_queue_handle.count;
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(struct core_drawing) * drawing_count,
                    &core->drawing_queue[0]);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, drawing_count);
    queue_reset(&core->drawing_queue_handle);
}

void core_render_lines(struct core *core)
{
    glBindBuffer(GL_ARRAY_BUFFER, core->line_instance_vertex_buffer_object);
    int line_count = core->line_queue_handle.count;
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct core_line) * line_count,
                    &core->line_queue[0]);
    glDrawArraysInstanced(GL_LINES, 0, 2, line_count);
    queue_reset(&core->line_queue_handle);
}

void core_update_window(SDL_Window *window) { SDL_GL_SwapWindow(window); }

void core_update_viewport(struct core *core, int viewport_width,
                          int viewport_height)
{
    glViewport(0, 0, viewport_width, viewport_height);
    core->viewport_width = viewport_width;
    core->viewport_height = viewport_height;
}

void core_offscreen_rendering_begin(struct core *core,
                                    struct core_texture *target_texture)
{
    // bind frame buffer object
    glBindFramebuffer(GL_FRAMEBUFFER, core->frame_buffer_object);

    // attatch target texture to bound frame buffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           target_texture->id, 0);
}

void core_offscreen_rendering_end(void)
{
    // unbind frame buffer object, bind to the default fbo
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
