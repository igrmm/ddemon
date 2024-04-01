#ifndef MAP_H
#define MAP_H

#include "SDL.h" // IWYU pragma: keep //clangd

#define MAP_TILE_SIZE 32
#define MAP_TILES_X_MAX 350
#define MAP_TILES_Y_MAX 100
#define MAP_TILE_LAYERS_MAX 4

struct map_tile_layer {
    Uint8 tileset_index_x;
    Uint8 tileset_index_y;
    Uint8 enabled;
};

struct map_tile {
    Uint8 collidable;
    struct map_tile_layer layers[MAP_TILE_LAYERS_MAX];
};

struct map {
    struct map_tile tiles[MAP_TILES_X_MAX][MAP_TILES_Y_MAX];
};

void map_new(struct map *map);
int map_to_file(struct map *map, const char *path);
int map_from_file(struct map *map, const char *path);

#endif
