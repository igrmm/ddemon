#include "nk.h"

#include "../ecs.h"
#include "app.h"
#include "prop.h"

enum prop_entity_type {
    PROP_ENTITY_WAYPOINT = 0,
    PROP_ENTITY_PORTAL,
    PROP_ENTITY_TOTAL
};

static const char *prop_entity_type_strings[] = {
    [PROP_ENTITY_WAYPOINT] = "waypoint",
    [PROP_ENTITY_PORTAL] = "portal"};

static const enum prop_entity_type prop_entity_from_tag_table[] = {
    [CMP_TAG_WAYPOINT] = PROP_ENTITY_WAYPOINT,
    [CMP_TAG_PORTAL] = PROP_ENTITY_PORTAL};

static const enum component_tag_type prop_entity_to_tag_table[] = {
    [PROP_ENTITY_WAYPOINT] = CMP_TAG_WAYPOINT,
    [PROP_ENTITY_PORTAL] = CMP_TAG_PORTAL};

static void prop_entity_waypoint_edit(Uint16 entity, struct app *app);

static void (*prop_entity_edit_table[PROP_ENTITY_TOTAL])(Uint16 entity,
                                                         struct app *app) = {
    // clang-format off
    prop_entity_waypoint_edit
    // clang-format on
};

static void prop_entity_waypoint_edit(Uint16 entity, struct app *app)
{
    struct nk_context *nk_ctx = app->nk_ctx;
    struct component *component =
        ecs_get_component(app->ecs, CMP_TYPE_WAYPOINT, entity);
    struct component_waypoint *cmp_waypoint = &component->data.waypoint;

    nk_label(nk_ctx, "waypoint", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(
        nk_ctx, NK_EDIT_SIMPLE, cmp_waypoint->waypoint_name,
        SDL_arraysize(cmp_waypoint->waypoint_name), nk_filter_default);

    nk_label(app->nk_ctx, "map", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(
        nk_ctx, NK_EDIT_SIMPLE, cmp_waypoint->map_name,
        SDL_arraysize(cmp_waypoint->map_name), nk_filter_default);
}

static void prop_edit_entity(struct app *app)
{
    struct nk_context *nk_ctx = app->nk_ctx;

    // get the selected entity
    struct ecs_node *node = NULL;
    Uint16 entity;
    while (ecs_table_iterate_entities(app->selected_entities, &node, &entity))
        /* nothing */;

    // get current entity type
    struct component *component =
        ecs_get_component(app->ecs, CMP_TYPE_TAG, entity);
    struct component_tag *cmp_tag = &component->data.tag;
    enum prop_entity_type current_entity_type =
        prop_entity_from_tag_table[cmp_tag->type];

    nk_layout_row_dynamic(nk_ctx, 25, 2);

    // show entity "number"
    char entity_str[6] = "0"; // Uint16 max value = 65535
    int entity_str_size = SDL_arraysize(entity_str);
    nk_label(nk_ctx, "entity", NK_TEXT_LEFT);
    SDL_snprintf(entity_str, entity_str_size, "%i", entity);
    nk_edit_string_zero_terminated(nk_ctx, NK_EDIT_READ_ONLY, entity_str,
                                   entity_str_size, nk_filter_decimal);

    // show combobox with entity types
    enum prop_entity_type new_entity_type;
    nk_label(nk_ctx, "entity_type", NK_TEXT_LEFT);
    new_entity_type =
        nk_combo(nk_ctx, prop_entity_type_strings, PROP_ENTITY_TOTAL,
                 current_entity_type, 25, nk_vec2i(150, 150));

    // modify type of selected entity
    if (current_entity_type != new_entity_type) {
        // TODO: remove all components from entity (ecs)
        // TODO: add new component_tag of new type
        // TODO: "create" new entity with all components
        cmp_tag->type = prop_entity_to_tag_table[new_entity_type];
        SDL_Log("NEW ENTITY TYPE: %i", new_entity_type); // debug info
    }

    // edit the selected entity
    (*prop_entity_edit_table[new_entity_type])(entity, app);
}

void prop_window(struct app *app)
{
    struct nk_context *nk_ctx = app->nk_ctx;
    int window_flags = app->window_flags;

    if (nk_begin(nk_ctx, "properties", nk_rect(20, 365, 200, 200),
                 window_flags)) {
        if (ecs_table_get_count(app->selected_entities) == 1) {
            prop_edit_entity(app);
        } else {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            nk_label(nk_ctx, "no selection", NK_TEXT_LEFT);
        }
    } else {
        app->show_prop_window = 0;
    }
    nk_end(nk_ctx);
}
