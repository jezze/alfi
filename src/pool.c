#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "widgets.h"

#define NWIDGETS                        512

static struct alfi_widget widgets[NWIDGETS];
static struct list freelist;
static struct list usedlist;

static struct alfi_widget *prev(struct list *list, struct alfi_widget *widget)
{

    struct list_item *current = (widget) ? widget->item.prev : list->tail;

    return (current) ? current->data : 0;

}

static struct alfi_widget *next(struct list *list, struct alfi_widget *widget)
{

    struct list_item *current = (widget) ? widget->item.next : list->head;

    return (current) ? current->data : 0;

}

struct alfi_widget *pool_prev(struct alfi_widget *widget)
{

    return prev(&usedlist, widget);

}

struct alfi_widget *pool_next(struct alfi_widget *widget)
{

    return next(&usedlist, widget);

}

struct alfi_widget *pool_findbyname(char *name)
{

    struct alfi_widget *widget = 0;

    while ((widget = pool_next(widget)))
    {

        if (!strlen(widget->header.id.name))
            continue;

        if (!strcmp(widget->header.id.name, name))
            return widget;

    }

    return 0;

}

struct alfi_widget *pool_nextchild(struct alfi_widget *widget, struct alfi_widget *parent)
{

    while ((widget = pool_next(widget)))
    {

        if (pool_findbyname(widget->header.in.name) == parent)
            return widget;

    }

    return 0;

}

struct alfi_widget *pool_create(void)
{

    struct list_item *item = list_pickhead(&freelist);

    list_add(&usedlist, item);

    return item->data;

}

void pool_destroy(struct alfi_widget *widget)
{

    list_move(&freelist, &usedlist, &widget->item);

}

char *pool_allocate(char *string, unsigned int size, unsigned int count, char *content)
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

void pool_setup(void)
{

    unsigned int i;

    for (i = 0; i < NWIDGETS; i++)
    {

        list_inititem(&widgets[i].item, &widgets[i]);
        list_add(&freelist, &widgets[i].item);

    }

}

