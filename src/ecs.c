#include "SDL.h" // IWYU pragma: keep //clangd

#include "components.h"
#include "ecs.h"

#define ECS_BUFSIZ 1000

union ecs_node_data {
    struct component component;
    Uint16 entity;
};

struct ecs_node {
    struct ecs_node *next;
    struct ecs_node *prev;
    union ecs_node_data data;
};

struct ecs_list {
    struct ecs_node *head;
    struct ecs_node *tail;
};

struct ecs_table {
    struct ecs_node buffer[ECS_BUFSIZ];
    struct ecs_list list;
    Uint16 count;
};

struct ecs {
    struct ecs_table entity_pool;
    struct ecs_table entities;
    struct ecs_table components[CMP_TYPE_TOTAL];
    Uint16 player_entity;
};

static void ecs_list_add(struct ecs_list *list, struct ecs_node *node)
{
    if (list->head == NULL) {
        list->head = node;
    }

    if (list->tail != NULL) {
        list->tail->next = node;
        node->prev = list->tail;
    }

    list->tail = node;
}

static void ecs_list_remove(struct ecs_list *list, struct ecs_node *node)
{
    if (list->tail == node && list->head != node) {
        node->prev->next = NULL;
        list->tail = list->tail->prev;
    }

    if (list->head == node && list->tail != node) {
        node->next->prev = NULL;
        list->head = list->head->next;
    }

    if (node->next != NULL && node->prev != NULL) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    if (list->head == node && list->tail == node) {
        list->head = list->tail = NULL;
    }
}

/* Retrieves and removes the last entity node, or returns zero if empty. */
static Uint16 ecs_table_entity_poll_last(struct ecs_table *entity_table)
{
    struct ecs_list *list = &entity_table->list;
    if (list->tail == NULL)
        return 0;
    struct ecs_node *node = list->tail;
    Uint16 entity = node->data.entity;
    ecs_list_remove(list, node);
    entity_table->count--;
    return entity;
}

/* Appends the specified entity node to the end of this table. */
static void ecs_table_entity_add(struct ecs_table *entity_table, Uint16 entity)
{
    struct ecs_node *node = &entity_table->buffer[entity];
    node->data.entity = entity;
    struct ecs_list *list = &entity_table->list;
    ecs_list_add(list, node);
    entity_table->count++;
}

/* Removes the specified entity node of this table. */
static void ecs_table_entity_remove(struct ecs_table *entity_table,
                                    Uint16 entity)
{
    struct ecs_node *node = &entity_table->buffer[entity];
    struct ecs_list *list = &entity_table->list;
    ecs_list_remove(list, node);
    entity_table->count--;
}

struct ecs *ecs_create(void)
{
    struct ecs *ecs = SDL_calloc(1, sizeof(*ecs));
    if (ecs == NULL)
        return NULL;
    // fill entity pool
    for (Uint16 entity = 0; entity < ECS_BUFSIZ; entity++)
        ecs_table_entity_add(&ecs->entity_pool, entity);
    ecs->player_entity = ecs_create_entity(ecs);
    return ecs;
}

void ecs_remove_entity(struct ecs *ecs, Uint16 entity)
{
    size_t types = SDL_arraysize(ecs->components);
    for (size_t type = 0; type < types; type++)
        ecs_remove_component(ecs, type, entity);
    ecs_table_entity_remove(&ecs->entities, entity);
    ecs_table_entity_add(&ecs->entity_pool, entity);
}

Uint16 ecs_create_entity(struct ecs *ecs)
{
    Uint16 entity = ecs_table_entity_poll_last(&ecs->entity_pool);
    ecs_table_entity_add(&ecs->entities, entity);
    return entity;
}

void ecs_add_component(struct ecs *ecs, struct component component_added)
{
    Uint16 entity = component_added.entity;
    enum component_type type = component_added.type;
    struct ecs_table *table = &ecs->components[type];
    struct ecs_node *node = &table->buffer[entity];
    struct ecs_list *list = &ecs->components[type].list;
    struct component *component = &node->data.component;
    *component = component_added;
    ecs_list_add(list, node);
    table->count++;
}

void ecs_remove_component(struct ecs *ecs, enum component_type type,
                          Uint16 entity)
{
    struct ecs_table *table = &ecs->components[type];
    struct ecs_node *node = &table->buffer[entity];
    struct ecs_list *list = &ecs->components[type].list;
    ecs_list_remove(list, node);
    struct component *component = &node->data.component;
    *component = (struct component){0};
    table->count--;
}

struct component *ecs_get_component(struct ecs *ecs, enum component_type type,
                                    Uint16 entity)
{
    struct ecs_node *node = &ecs->components[type].buffer[entity];
    struct component *component = &node->data.component;
    if (component->alive == SDL_FALSE)
        return NULL;
    return component;
}

static int ecs_pool_node(struct ecs_list *list, struct ecs_node **node)
{
    /** if the list is empty, stop the polling */
    if (list->head == NULL)
        return 0;

    /** this is the start of the polling, the given node will become the first
     * item of the list, returns 1 to indicate that this item is not NULL */
    if ((*node) == NULL) {
        *node = list->head;
        return 1;
    }

    /** node becomes next item on the list */
    *node = (*node)->next;

    /** this is the end of the polling, NULL node means that the end of the list
     * was reached */
    if ((*node) == NULL)
        return 0;

    /** returns 1 to indicate that "next" item is not NULL */
    return 1;
}

int ecs_poll_entity(struct ecs *ecs, struct ecs_node **node, Uint16 *entity)
{
    struct ecs_list *list = &ecs->entities.list;
    int ret = ecs_pool_node(list, node);
    if (ret)
        *entity = (*node)->data.entity;
    return ret;
}

int ecs_poll_component(struct ecs *ecs, struct ecs_node **node,
                       struct component **component, enum component_type type)
{
    struct ecs_list *list = &ecs->components[type].list;
    int ret = ecs_pool_node(list, node);
    if (ret)
        *component = &(*node)->data.component;
    return ret;
}
