#include "list.h"

void list_add(struct list *list, struct list_item *item)
{

    item->prev = list->tail;
    item->next = 0;

    if (list->head)
        list->tail->next = item;
    else
        list->head = item;

    list->tail = item;
    list->count++;

}

void list_remove(struct list *list, struct list_item *item)
{

    if (list->head == item)
        list->head = item->next;

    if (list->tail == item)
        list->tail = item->prev;

    if (list->head)
        list->head->prev = 0;

    if (list->tail)
        list->tail->next = 0;

    if (item->next)
        item->next->prev = item->prev;

    if (item->prev)
        item->prev->next = item->next;

    item->next = 0;
    item->prev = 0;
    list->count--;

}

void list_move(struct list *to, struct list *from, struct list_item *item)
{

    list_remove(from, item);
    list_add(to, item);

}

struct list_item *list_pickhead(struct list *list)
{

    struct list_item *item = list->head;

    if (item)
        list_remove(list, item);

    return item;

}

struct list_item *list_picktail(struct list *list)
{

    struct list_item *item = list->tail;

    if (item)
        list_remove(list, item);

    return item;

}

void list_inititem(struct list_item *item, void *data)
{

    item->next = 0;
    item->prev = 0;
    item->data = data;

}

