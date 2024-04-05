#ifndef ECS_H
#define ECS_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "components.h"

struct ecs;
struct ecs_node;
struct ecs_table;

struct ecs *ecs_create(void);
struct ecs_table *ecs_create_table(void);
void ecs_remove_entity(struct ecs *ecs, Uint16 entity);
Uint16 ecs_create_entity(struct ecs *ecs);
void ecs_add_component(struct ecs *ecs, struct component component_added);
void ecs_remove_component(struct ecs *ecs, enum component_type type,
                          Uint16 entity);
struct component *ecs_get_component(struct ecs *ecs, enum component_type type,
                                    Uint16 entity);
int ecs_poll_entity(struct ecs *ecs, struct ecs_node **node, Uint16 *entity);
int ecs_poll_component(struct ecs *ecs, struct ecs_node **node,
                       struct component **component, enum component_type type);

#endif
