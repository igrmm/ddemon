#define GLAD_GL_IMPLEMENTATION
#include "../external/glad.h"

#include <SDL3/SDL.h>

#include "core.h"

bool core_setup(struct core *core, const char *window_title, int window_width,
                int window_height, int window_flag)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

#if defined(SDL_PLATFORM_ANDROID)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_ES);
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

    // malloc memory for drawing pool
    core->drawing_pool =
        SDL_malloc(CORE_DRAWING_POOL_SIZE * sizeof(struct core_drawing));
    if (core->drawing_pool == NULL) {
        SDL_Log("Error in core_setup(): malloc failed (drawing_pool)");
        return false;
    }

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

    // setup instance vbo
    glGenBuffers(1, &core->instance_vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->instance_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(struct core_drawing) * CORE_DRAWING_POOL_SIZE, NULL,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind instance_vbo

    // setup vertices/indices of quad for the template drawing
    float vertices[] = {
        // w     h     <- identify if this is a "width" or "height" vertex
        1.0f, 1.0f, // top right
        1.0f, 0.0f, // bottom right
        0.0f, 0.0f, // bottom left
        0.0f, 1.0f, // top left
    };
    Uint32 indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // setup vbo for the quad template
    glGenBuffers(1, &core->vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo

    // setup ebo for the quad template
    glGenBuffers(1, &core->element_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, core->element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

    // put the vertex atrib of the quad template vbo in the vao
    glBindBuffer(GL_ARRAY_BUFFER, core->vertex_buffer_object);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo

    // put the vertex atrib of the instance vbo in the vao
    glBindBuffer(GL_ARRAY_BUFFER, core->instance_vertex_buffer_object);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (void *)0);
    glVertexAttribDivisor(1, 1); // create instance
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (void *)(4 * sizeof(float)));
    glVertexAttribDivisor(2, 1); // create instance
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (void *)(8 * sizeof(float)));
    glVertexAttribDivisor(3, 1);      // create instance
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind instance vbo

    // bind ebo and instance vbo to be used throughout the whole program
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, core->element_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->instance_vertex_buffer_object);

    return true;
}

void core_shutdown(struct core *core)
{
    if (core->frame_buffer_object > 0)
        glDeleteFramebuffers(1, &core->frame_buffer_object);

    if (core->vertex_array_object > 0)
        glDeleteVertexArrays(1, &core->vertex_array_object);

    if (core->instance_vertex_buffer_object > 0)
        glDeleteBuffers(1, &core->instance_vertex_buffer_object);

    if (core->vertex_buffer_object > 0)
        glDeleteBuffers(1, &core->vertex_buffer_object);

    if (core->element_buffer_object > 0)
        glDeleteBuffers(1, &core->element_buffer_object);

    SDL_free(core->drawing_pool);
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
                                        const Uint8 *texture_data)
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
                 opengl_format, GL_UNSIGNED_BYTE, texture_data);
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

static bool core_get_drawing_instance(struct core *core, int *instance)
{
    // check if there is available instances in the pool
    if (core->drawing_queue_size + 1 > CORE_DRAWING_POOL_SIZE) {
        SDL_Log(
            "Error getting drawing instance: no instances available in pool.");
        return false;
    }

    // obtain instance from the pool
    core->drawing_queue_size++;
    *instance = core->drawing_queue_size - 1;
    return true;
}

bool core_add_drawing_color_tex(struct core *core, const SDL_FRect *tex_region,
                                const SDL_FRect *src_rect,
                                const SDL_FRect *dst_rect,
                                const struct core_color *color)
{
    int instance;
    if (!core_get_drawing_instance(core, &instance))
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
    core->drawing_pool[instance] = (struct core_drawing){
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

void core_draw_queue(struct core *core)
{
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(struct core_drawing) * core->drawing_queue_size,
                    &core->drawing_pool[0]);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0,
                            core->drawing_queue_size);

    // return all instances to the pool
    core->drawing_queue_size = 0;
}

void core_update_window(SDL_Window *window) { SDL_GL_SwapWindow(window); }

void core_update_viewport(struct core *core, int viewport_width,
                          int viewport_height)
{
    glViewport(0, 0, viewport_width, viewport_height);
    core->viewport_width = viewport_width;
    core->viewport_height = viewport_height;
}

void core_restore_gl_state(struct core *core)
{
    glEnable(GL_BLEND);
    glBindVertexArray(core->vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, core->instance_vertex_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, core->element_buffer_object);
    glUseProgram(core->current_shader);
    glBindTexture(GL_TEXTURE_2D, core->current_texture.id);
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
