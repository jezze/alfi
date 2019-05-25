#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "box.h"
#include "widgets.h"
#include "parser.h"

#define NWIDGETS                        512
#define STRINGTABLESIZE                 8192

static struct alfi_widget widgets[NWIDGETS];
static char strings[STRINGTABLESIZE];
static struct parser parser;
static struct list freelist;
static struct list usedlist;

static struct alfi_widget *nextwidget(struct list *list, struct alfi_widget *widget)
{

    struct list_item *current = (widget) ? widget->item.next : list->head;

    return (current) ? current->data : 0;

}

static struct alfi_widget *findwidgetbyname(struct list *list, unsigned int group, char *name)
{

    struct alfi_widget *widget = 0;

    while ((widget = nextwidget(list, widget)))
    {

        if (widget->group != group)
            continue;

        if (!widget->id.name)
            continue;

        if (!strcmp(widget->id.name, name))
            return widget;

    }

    return 0;

}

static struct alfi_widget *nextchild(struct list *list, struct alfi_widget *widget, struct alfi_widget *parent)
{

    while ((widget = nextwidget(list, widget)))
    {

        if (widget->group != parent->group)
            continue;

        if (!widget->in.name)
            continue;

        if (findwidgetbyname(list, widget->group, widget->in.name) == parent)
            return widget;

    }

    return 0;

}

static struct alfi_widget *parser_find(char *name, unsigned int group)
{

    return findwidgetbyname(&usedlist, group, name);

}

static struct alfi_widget *parser_create(unsigned int type, unsigned int group, char *in)
{

    struct list_item *item = list_pickhead(&freelist);
    struct alfi_widget *widget = item->data;

    widget->type = type;
    widget->group = group;
    widget->in.name = in;

    list_add(&usedlist, item);

    return widget;

}

static void parser_destroy(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = nextchild(&usedlist, child, widget)))
    {

        parser_destroy(child);

        child = 0;

    }

    list_move(&freelist, &usedlist, &widget->item);

}

static void parser_fail(unsigned int line)
{

    printf("Parsing failed on line %u\n", line);
    exit(EXIT_FAILURE);

}

static void load(void)
{

    unsigned int i;

    for (i = 0; i < NWIDGETS; i++)
    {

        list_inititem(&widgets[i].item, &widgets[i]);
        list_add(&freelist, &widgets[i].item);

    }

    parser.string.data = strings;
    parser.string.count = STRINGTABLESIZE;
    parser.string.offset = 0;
    parser.find = parser_find;
    parser.create = parser_create;
    parser.destroy = parser_destroy;
    parser.fail = parser_fail;

}

static void loadbase(unsigned int group)
{

    struct alfi_widget *widget;

    widget = parser_create(ALFI_WIDGET_WINDOW, group, "");

    if (widget)
    {

        widget->id.name = "window";
        widget->data.window.label.content = "undefined";

    }

    widget = parser_create(ALFI_WIDGET_VSTACK, group, "window");

    if (widget)
    {

        widget->id.name = "main";
        widget->data.vstack.halign.direction = ALFI_HALIGN_LEFT;
        widget->data.vstack.valign.direction = ALFI_VALIGN_TOP;

    }

}

int main(int argc, char **argv)
{

    char data[4096];

    load();
    loadbase(1);

    parser.expr.data = data;
    parser.expr.count = fread(data, 1, 4096, stdin);
    parser.expr.offset = 0;
    parser.expr.line = 0;
    parser.expr.linebreak = 0;
    parser.expr.inside = 0;

    parse(&parser, 1, "main");

    return 0;

}

