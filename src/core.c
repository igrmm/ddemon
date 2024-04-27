#define GLAD_GL_IMPLEMENTATION
#include "../external/glad.h"

#include "SDL.h" // IWYU pragma: keep //clangd

#include "core.h"

/**
 *  SDL VARIABLES
 */
static SDL_Window *window;
static SDL_GLContext *ctx;

/**
 *  OPENGL VARIABLES
 */
static GLuint vao;
static GLuint tile_vbo;
static GLuint tile_ebo;
static GLuint tile_instance_vbo;
static GLuint current_shader;
static GLuint current_texture;
static GLuint current_ebo;

/**
 *  APP CONFIG
 */
#define WINDOW_NAME "DDEMON"
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define TILE_SIZE 32
#define LAYER_NUM 6
#define BUFSIZ_TILE_X 61 // ROUND(WINDOW_WIDTH/TILE_SIZE)+1
#define BUFSIZ_TILE_Y 35 // ROUND(WINDOW_HEIGHT/TILE_SIZE)+1

/**
 *  Considering:
 *  - Target resolution: 1920x1080
 *  - Target tile size: 32
 *  - Target layer number: 6
 *  - A buffer of 1 column and 1 row on the edges
 *
 *  Number of tiles on x will be: 1920/32 + 1 = 61
 *  Number of tiles on y will be: 1080/32 + 1 = 35
 *  Total number of tiles/transformations: 61*35*6 = 12810
 */
#define TILE_TRANSFORM_NUM 12810

struct tile_transform {
    float x, y;
    float tex_x, tex_y;
};

static struct tile_transform tile_transforms[TILE_TRANSFORM_NUM];

static float normalize_x(float x, float viewport_width)
{
    return x / viewport_width * 2 - 1;
}

static float normalize_y(float y, float viewport_height)
{
    return y / viewport_height * 2 - 1;
}

static void core_initialize_map_renderer(void)
{
    // initialize tile transformations
    int n = 0;
    const int tileset_tile_num_x = 6;
    for (int l = 0; l < LAYER_NUM; l++) {
        for (int x = 0; x < BUFSIZ_TILE_X; x += 1) {
            for (int y = 0; y < BUFSIZ_TILE_Y; y += 1) {
                float offsetx = (float)x / WINDOW_WIDTH * 2 * TILE_SIZE;
                float offsety = (float)y / WINDOW_HEIGHT * 2 * TILE_SIZE;
                float tloffsetx = (float)l / tileset_tile_num_x;
                tile_transforms[n] =
                    (struct tile_transform){offsetx, offsety, tloffsetx, 0};
                n++;
            }
        }
    }

    // setup instance vbo
    glGenBuffers(1, &tile_instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tile_instance_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(struct tile_transform) * TILE_TRANSFORM_NUM,
                 &tile_transforms[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind instance_vbo

    // setup vertices/indices of quad for the tile template
    SDL_FPoint top_right = {normalize_x(TILE_SIZE, WINDOW_WIDTH),
                            normalize_y(TILE_SIZE, WINDOW_HEIGHT)};
    SDL_FPoint bottom_right = {normalize_x(TILE_SIZE, WINDOW_WIDTH),
                               normalize_y(0, WINDOW_HEIGHT)};
    SDL_FPoint bottom_left = {normalize_x(0, WINDOW_WIDTH),
                              normalize_y(0, WINDOW_HEIGHT)};
    SDL_FPoint top_left = {normalize_x(0, WINDOW_WIDTH),
                           normalize_y(TILE_SIZE, WINDOW_HEIGHT)};
    float tloffsetx = 1.0f / 6.0f;
    float vertices[] = {
        // positions                    // texture coords
        top_right.x,    top_right.y,    tloffsetx, 1.0f, // top right
        bottom_right.x, bottom_right.y, tloffsetx, 0.0f, // bottom right
        bottom_left.x,  bottom_left.y,  0.0f,      0.0f, // bottom left
        top_left.x,     top_left.y,     0.0f,      1.0f  // top left
    };
    Uint32 indices[] = {
        0, 1, 3, // first Triangle
        1, 2, 3  // second Triangle
    };

    // setup vbo for the tile template
    glGenBuffers(1, &tile_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tile_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind tile vbo

    // setup ebo for the tile template
    glGenBuffers(1, &tile_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tile_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind tile ebo

    // put the vertex atrib of the tile template vbo in the vao
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tile_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind tile vbo

    // put the vertex atrib of the tile instance vbo in the vao
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, tile_instance_vbo);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind instance vbo

    // create the instances
    glVertexAttribDivisor(1, 1);
}

static int core_initialize_gl(void)
{
    ctx = SDL_GL_CreateContext(window);

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
        return -1;
    }
    SDL_Log("GLAD initialization success.");
    SDL_Log("PLATFORM: %s, PROFILE: %i, VERSION: %i", platform, profile,
            version);

    // vsync off so we can see fps
    SDL_GL_SetSwapInterval(0);

    // enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    return 0;
}

int core_setup(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    window =
        SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                         SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    if (core_initialize_gl() < 0)
        return -1;

    core_initialize_map_renderer();

    return 0;
}

void core_shutdown(void)
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &tile_vbo);
    glDeleteBuffers(1, &tile_ebo);
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
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

void core_delete_texture(const Uint32 *texture)
{
    glDeleteTextures(1, texture);
}

Uint32 core_create_texture(int width, int height, const Uint8 *texture_data)
{
    Uint32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, texture_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void core_clear_screen(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void core_use_shader(Uint32 shader)
{
    if (current_shader != shader) {
        glUseProgram(shader);
        current_shader = shader;
    }
}

void core_draw_map(Uint32 tilemap)
{
    if (current_texture != tilemap) {
        glBindTexture(GL_TEXTURE_2D, tilemap);
        current_texture = tilemap;
    }

    if (current_ebo != tile_ebo) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tile_ebo);
        current_ebo = tile_ebo;
    }

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0,
                            TILE_TRANSFORM_NUM);
}

void core_update_window(void) { SDL_GL_SwapWindow(window); }
