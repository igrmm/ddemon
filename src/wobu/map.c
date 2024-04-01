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

void map_new(struct map *map)
{
    for (int i = 0; i < MAP_TILES_X_MAX; i++) {
        for (int j = 0; j < MAP_TILES_Y_MAX; j++) {
            map->tiles[i][j] =
                (struct map_tile){0,
                                  {(struct map_tile_layer){0, 0, 0},
                                   (struct map_tile_layer){0, 0, 0},
                                   (struct map_tile_layer){0, 0, 0},
                                   (struct map_tile_layer){0, 0, 0}}};
        }
    }
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

int map_from_file(struct map *map, const char *path)
{
    map_new(map);
    Uint32 time_start = SDL_GetTicks64();
    // DESERIALIZE JSON STRING
    // todo: json error handling
    struct json_value_s *json = util_json_from_file(path);
    if (json == NULL) {
        SDL_Log("Error parsing json map.");
        return -1;
    }

    struct json_object_s *json_root = (struct json_object_s *)json->payload;

    // START LOADING TILES
    struct json_object_element_s *element = json_root->start;
    if (SDL_strcmp("tiles", element->name->string) != 0) {
        SDL_Log("Json Object is not 'tiles', found: %s", element->name->string);
        SDL_free(json);
        return -1;
    }

    struct json_array_s *json_tiles = json_value_as_array(element->value);
    size_t total_tiles = json_tiles->length;
    struct json_array_element_s *json_tile = json_tiles->start;
    struct map_tile *tile = NULL;

    // loop through all tiles
    // format: [X,Y,[COLLIDABLE,[[LAYER,TILESET_IDX,TILESET_IDY]]]]
    while (json_tile != NULL) {
        struct json_array_s *json_array_tile =
            json_value_as_array(json_tile->value);

        // index x
        struct json_array_element_s *json_x = json_array_tile->start;
        struct json_number_s *json_number_x =
            json_value_as_number(json_x->value);
        int x = SDL_strtol(json_number_x->number, NULL, 10);

        // index y
        struct json_array_element_s *json_y = json_x->next;
        struct json_number_s *json_number_y =
            json_value_as_number(json_y->value);
        int y = SDL_strtol(json_number_y->number, NULL, 10);

        tile = &map->tiles[x][y];

        // map_tile
        struct json_array_element_s *json_map_tile = json_y->next;
        struct json_array_s *json_array_map_tile =
            json_value_as_array(json_map_tile->value);

        // map_tile.collidable
        struct json_array_element_s *json_collidable =
            json_array_map_tile->start;
        struct json_number_s *json_number_collidable =
            json_value_as_number(json_collidable->value);
        int collidable = SDL_strtol(json_number_collidable->number, NULL, 10);
        tile->collidable = collidable;

        // map_tile.layers
        struct json_array_element_s *json_layers = json_collidable->next;
        struct json_array_s *json_array_layers =
            json_value_as_array(json_layers->value);

        // loop through layers
        struct json_array_element_s *json_layer = json_array_layers->start;
        while (json_layer != NULL) {
            struct json_array_s *json_array_layer =
                json_value_as_array(json_layer->value);

            // layer
            struct json_array_element_s *json_layer_number =
                json_array_layer->start;
            struct json_number_s *json_number_layer_number =
                json_value_as_number(json_layer_number->value);
            int layer = SDL_strtol(json_number_layer_number->number, NULL, 10);
            tile->layers[layer].enabled = 1;

            // tileset index x
            struct json_array_element_s *json_tileset_index_x =
                json_layer_number->next;
            struct json_number_s *json_number_tileset_index_x =
                json_value_as_number(json_tileset_index_x->value);
            int tileset_index_x =
                SDL_strtol(json_number_tileset_index_x->number, NULL, 10);
            tile->layers[layer].tileset_index_x = tileset_index_x;

            // tileset index y
            struct json_array_element_s *json_tileset_index_y =
                json_tileset_index_x->next;
            struct json_number_s *json_number_tileset_index_y =
                json_value_as_number(json_tileset_index_y->value);
            int tileset_index_y =
                SDL_strtol(json_number_tileset_index_y->number, NULL, 10);
            tile->layers[layer].tileset_index_y = tileset_index_y;

            json_layer = json_layer->next;
        }

        json_tile = json_tile->next;
    }
    SDL_free(json);
    SDL_Log("map loaded from file with %zu tiles in %i ms.", total_tiles,
            (int)(SDL_GetTicks64() - time_start));
    return 0;
}
