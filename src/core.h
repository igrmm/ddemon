/**
 *  \file core.h
 *
 *  Setup and shutdown SDL library.
 *  Setup OpenGL.
 */

#ifndef CORE_H
#define CORE_H

#include "SDL.h" // IWYU pragma: keep //clangd

int core_setup(void);
void core_shutdown(void);
void core_delete_shader(Uint32 shader);
Uint32 core_create_shader(const char *vert_src, const char *frag_src,
                          int *status, char *log, size_t log_size);
void core_delete_texture(const Uint32 *texture);
Uint32 core_create_texture(int width, int height, const Uint8 *texture_data);
void core_clear_screen(float r, float g, float b, float a);
void core_use_shader(Uint32 shader);
void core_draw_map(Uint32 tilemap);
void core_update_window(void);

#endif
