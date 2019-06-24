#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "box.h"
#include "widgets.h"
#include "parser.h"
#include "call.h"
#include "pool.h"

static struct parser parser;

static struct alfi_widget *parser_find(char *name, unsigned int group)
{

    return pool_findbyname(group, name);

}

static char *parser_createstring(unsigned int size)
{

    return malloc(size);

}

static void parser_destroystring(char *string)
{

    free(string);

}

static struct alfi_widget *parser_create(unsigned int type, unsigned int group, char *in)
{

    struct alfi_widget *widget = pool_create();

    widget->type = type;
    widget->group = group;
    widget->in.name = in;

    return widget;

}

static void parser_destroy(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        parser_destroy(child);

        child = 0;

    }

    call_destroy(widget);
    pool_destroy(widget);

}

static void parser_fail(unsigned int line)
{

    printf("Parsing failed on line %u\n", line);
    exit(EXIT_FAILURE);

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
    int count;

    pool_init();
    parser_init(&parser, parser_fail, parser_find, parser_create, parser_destroy, parser_createstring, parser_destroystring);
    loadbase(1);

    while ((count = read(0, data, 4096)))
        parser_parsedata(&parser, 1, "main", count, data);

    return 0;

}

