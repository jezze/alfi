#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "widgets.h"

#define NWIDGETS                        512
#define NRESOURCES                      128

static struct widget widgets[NWIDGETS];
static struct resource resources[NRESOURCES];
static struct list freewidgets;
static struct list usedwidgets;
static struct list freeresources;
static struct list usedresources;

static struct widget *prevwidget(struct list *list, struct widget *widget)
{

    struct list_item *current = (widget) ? widget->item.prev : list->tail;

    return (current) ? current->data : 0;

}

static struct widget *nextwidget(struct list *list, struct widget *widget)
{

    struct list_item *current = (widget) ? widget->item.next : list->head;

    return (current) ? current->data : 0;

}

struct widget *pool_widget_prev(struct widget *widget)
{

    return prevwidget(&usedwidgets, widget);

}

struct widget *pool_widget_next(struct widget *widget)
{

    return nextwidget(&usedwidgets, widget);

}

struct widget *pool_widget_find(char *name)
{

    struct widget *widget = 0;

    while ((widget = pool_widget_next(widget)))
    {

        if (!strlen(widget->header.id.name))
            continue;

        if (!strcmp(widget->header.id.name, name))
            return widget;

    }

    return 0;

}

struct widget *pool_widget_nextchild(struct widget *widget, struct widget *parent)
{

    while ((widget = pool_widget_next(widget)))
    {

        if (pool_widget_find(widget->header.in.name) == parent)
            return widget;

    }

    return 0;

}

struct widget *pool_widget_create(void)
{

    struct list_item *item = list_pickhead(&freewidgets);

    if (!item)
        return 0;

    list_add(&usedwidgets, item);

    return item->data;

}

struct widget *pool_widget_destroy(struct widget *widget)
{

    list_move(&freewidgets, &usedwidgets, &widget->item);

    return 0;

}

static struct resource *nextresource(struct list *list, struct resource *resource)
{

    struct list_item *current = (resource) ? resource->item.next : list->head;

    return (current) ? current->data : 0;

}

struct resource *pool_resource_find(char *name)
{

    struct resource *resource = 0;

    while ((resource = nextresource(&usedresources, resource)))
    {

        if (!strlen(resource->urlinfo.url))
            continue;

        if (!strcmp(resource->urlinfo.url, name))
            return resource;

    }

    return 0;

}

struct resource *pool_resource_create(void)
{

    struct list_item *item = list_pickhead(&freeresources);

    if (!item)
        return 0;

    list_add(&usedresources, item);

    return item->data;

}

struct resource *pool_resource_destroy(struct resource *resource)
{

    list_move(&freeresources, &usedresources, &resource->item);

    return 0;

}

char *pool_allocate(unsigned int type, char *string, unsigned int size, unsigned int count, char *content)
{

    if (string)
    {

        free(string);

        string = 0;

    }

    if (size)
    {

        string = malloc(size);

        if (count)
            memcpy(string, content, count);

    }

    return string;

}

char *pool_string_create(unsigned int type, char *string, char *content)
{

    return pool_allocate(type, string, strlen(content) + 1, strlen(content) + 1, content);

}

char *pool_string_destroy(unsigned int type, char *string)
{

    return pool_allocate(type, string, 0, 0, 0);

}

void pool_setup(void)
{

    unsigned int i;

    for (i = 0; i < NWIDGETS; i++)
    {

        list_inititem(&widgets[i].item, &widgets[i]);
        list_add(&freewidgets, &widgets[i].item);

    }

    for (i = 0; i < NRESOURCES; i++)
    {

        list_inititem(&resources[i].item, &resources[i]);
        list_add(&freeresources, &resources[i].item);

    }

}

