#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <SDL3/SDL.h>

#define CMP_STRINGBUF_SIZE 24

enum component_type {
    CMP_TYPE_TAG = 0,
    CMP_TYPE_RECT,
    CMP_TYPE_RENDERABLE,
    CMP_TYPE_WAYPOINT,
    CMP_TYPE_TOTAL
};

enum component_tag_type { CMP_TAG_WAYPOINT = 0, CMP_TAG_PORTAL, CMP_TAG_TOTAL };

struct component_tag {
    enum component_tag_type type;
};

struct component_rect {
    SDL_FRect rect;
};

struct component_renderable {
    SDL_Texture *texture;
};

struct component_waypoint {
    char waypoint_name[CMP_STRINGBUF_SIZE];
    char map_name[CMP_STRINGBUF_SIZE];
};

union component_data {
    struct component_tag tag;
    struct component_rect rect;
    struct component_renderable renderable;
    struct component_waypoint waypoint;
};

struct component {
    enum component_type type;
    Uint32 entity;
    bool alive;
    struct component *next;
    struct component *prev;
    union component_data data;
};

#endif
