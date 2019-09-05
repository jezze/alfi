#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "style.h"
#include "widgets.h"
#include "parser.h"
#include "call.h"
#include "pool.h"

static struct parser parser;

static struct alfi_widget *parser_findwidget(char *name)
{

    return pool_findbyname(name);

}

static char *parser_allocate(char *string, unsigned int size, unsigned int count, char *content)
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

static char *parser_createstring(char *string, char *content)
{

    return parser.allocate(string, strlen(content) + 1, strlen(content) + 1, content);

}

static char *parser_destroystring(char *string)
{

    return parser.allocate(string, 0, 0, 0);

}

static struct alfi_widget *parser_createwidget(unsigned int type, char *in)
{

    struct alfi_widget *widget = pool_create();

    widget->type = type;
    widget->id.name = parser.createstring(widget->id.name, "");
    widget->in.name = parser.createstring(widget->in.name, in);

    return widget;

}

static struct alfi_widget *parser_destroywidget(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        child = parser_destroywidget(child);

    widget->id.name = parser.destroystring(widget->id.name);
    widget->in.name = parser.destroystring(widget->in.name);

    call_destroy(widget);
    pool_destroy(widget);

    return 0;

}

static void parser_fail(unsigned int line)
{

    printf("Parsing failed on line %u\n", line);
    exit(EXIT_FAILURE);

}

static void loadbase(void)
{

    struct alfi_widget *widget;

    widget = parser_createwidget(ALFI_WIDGET_WINDOW, "");

    if (widget)
    {

        widget->id.name = "window";
        widget->data.window.label.content = "undefined";

    }

    widget = parser_createwidget(ALFI_WIDGET_VSTACK, "window");

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
    parser_init(&parser, parser_fail, parser_findwidget, parser_createwidget, parser_destroywidget, parser_allocate, parser_createstring, parser_destroystring);
    loadbase();

    while ((count = read(0, data, 4096)))
        parser_parse(&parser, "main", count, data);

    return 0;

}

