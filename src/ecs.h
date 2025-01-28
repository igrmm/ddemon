#ifndef ECS_H
#define ECS_H

#include <SDL3/SDL.h>

#include "components.h"

/**
 * This are forward declarations for the functions in this module
 */
struct ecs;
struct ecs_node;
struct ecs_table;

/**
 * This functions manage struct ecs_table directly
 */
void ecs_clear_table(struct ecs_table *table);
void ecs_add_entity_to_table(struct ecs_table *entity_table, Uint16 entity);
void ecs_remove_entity_from_table(struct ecs_table *entity_table,
                                  Uint16 entity);
struct ecs_table *ecs_create_table(void);
int ecs_iterate_table_entities(struct ecs_table *entity_table,
                               struct ecs_node **node, Uint16 *entity);
Uint16 ecs_get_table_count(struct ecs_table *table);

/**
 * This functions manage ecs_table indirectly via struct ecs
 */
struct ecs *ecs_create(void);
void ecs_remove_entity(struct ecs *ecs, Uint16 entity);
Uint16 ecs_create_entity(struct ecs *ecs);
void ecs_add_component(struct ecs *ecs, struct component component_added);
void ecs_remove_component(struct ecs *ecs, enum component_type type,
                          Uint16 entity);
struct component *ecs_get_component(struct ecs *ecs, enum component_type type,
                                    Uint16 entity);
int ecs_iterate_entities(struct ecs *ecs, struct ecs_node **node,
                         Uint16 *entity);
int ecs_iterate_components(struct ecs *ecs, struct ecs_node **node,
                           struct component **component,
                           enum component_type type);

#endif
