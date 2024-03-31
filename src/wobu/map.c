#include "SDL.h" // IWYU pragma: keep //clangd

#include "map.h"
#include "util.h"

static int map_tile_compare(struct map_tile *tile1, struct map_tile *tile2)
{
    if (tile1->collidable != tile2->collidable)
        return -1;
    for (int layer = 0; layer < MAP_TILE_LAYERS_MAX; layer++) {
        if (tile1->layers[layer].tileset_index_x !=
            tile2->layers[layer].tileset_index_x)
            return -1;
        if (tile1->layers[layer].tileset_index_y !=
            tile2->layers[layer].tileset_index_y)
            return -1;
        if (tile1->layers[layer].enabled != tile2->layers[layer].enabled)
            return -1;
    }
    return 0;
}

static void map_remove_trailing_comma(char *json_str)
{
    size_t json_str_len = SDL_strlen(json_str);
    size_t comma_index = json_str_len - 1;
    if (json_str[comma_index] == ',')
        json_str[comma_index] = '\0';
}

int map_to_file(struct map *map, const char *path)
{
    Uint32 time_start = SDL_GetTicks64();
    char json_str[JSON_BUFSIZ] = {0};
    size_t json_str_sz = SDL_arraysize(json_str);

    SDL_RWops *file = SDL_RWFromFile(path, "w");
    if (file == NULL) {
        SDL_Log("map serialization error");
        return -1;
    }

    // BEGINNING OF JSON STRING
    if (util_strcat(json_str, json_str_sz, "{\n    \"tiles\": [") != 0) {
        SDL_Log("map serialization error");
        return -1;
    }

    // WRITE TILES TO JSON STRING
    struct map_tile null_tile = {0};
    struct map_tile *tile = NULL;
    for (int i = 0; i < MAP_TILES_X_MAX; i++) {
        for (int j = 0; j < MAP_TILES_Y_MAX; j++) {
            tile = &map->tiles[i][j];
            if (map_tile_compare(tile, &null_tile) == 0)
                continue;
            // clang-format off
            if (util_strcat(json_str, json_str_sz,
                        "[%i,%i,[%i,[",
                        i,
                        j,
                        tile->collidable) != 0) {
                SDL_Log("map serialization error");
                return -1;
            }
            for (int layer = 0; layer < MAP_TILE_LAYERS_MAX; layer++) {
                if (tile->layers[layer].enabled != 0) {
                    if (util_strcat(json_str, json_str_sz,
                                "[%i,%i,%i],",
                                layer,
                                tile->layers[layer].tileset_index_x,
                                tile->layers[layer].tileset_index_y) != 0) {
                        SDL_Log("map serialization error");
                        return -1;
                    }
                }
            }
            // clang-format on
            map_remove_trailing_comma(json_str);

            // END OF THIS TILE
            if (util_strcat(json_str, json_str_sz, "]]],") != 0) {
                SDL_Log("map serialization error");
                return -1;
            }
        }
    }
    map_remove_trailing_comma(json_str);

    // ENDING OF JSON STRING
    if (util_strcat(json_str, json_str_sz, "]\n}") != 0) {
        SDL_Log("map serialization error");
        return -1;
    }

    // WRITE JSON STRING TO FILE
    size_t json_str_len = SDL_strlen(json_str);
    for (size_t i = 0; i < json_str_len; i++)
        SDL_RWwrite(file, &json_str[i], sizeof(char), 1);
    SDL_RWclose(file);
    SDL_Log("map saved to file with %zu bytes in %i ms.", json_str_len,
            (int)(SDL_GetTicks64() - time_start));
    return 0;
}
