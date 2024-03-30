#include "map.h"

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
