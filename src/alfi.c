#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "style.h"
#include "widgets.h"
#include "parser.h"
#include "pool.h"

static struct parser parser;

static char *createstring(char *string, char *content)
{

    return pool_allocate(string, strlen(content) + 1, strlen(content) + 1, content);

}

static char *destroystring(char *string)
{

    return pool_allocate(string, 0, 0, 0);

}

static struct alfi_widget *parser_createwidget(unsigned int type, char *in)
{

    struct alfi_widget *widget = pool_create();

    memset(&widget->payload, 0, sizeof (union alfi_payload));
    memset(&widget->frame, 0, sizeof (union alfi_frame));

    widget->header.type = type;
    widget->header.id.name = createstring(widget->header.id.name, "");
    widget->header.in.name = createstring(widget->header.in.name, in);

    return widget;

}

static struct alfi_widget *parser_destroywidget(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        child = parser_destroywidget(child);

    widget->header.id.name = destroystring(widget->header.id.name);
    widget->header.in.name = destroystring(widget->header.in.name);

    pool_destroy(widget);

    return 0;

}

static void parser_fail(void)
{

    printf("Parsing failed on line %u\n", parser.expr.line + 1);
    exit(EXIT_FAILURE);

}

static void loadbase(void)
{

    struct alfi_widget *widget;

    widget = parser_createwidget(ALFI_WIDGET_WINDOW, "");

    if (widget)
    {

        widget->header.id.name = createstring(widget->header.id.name, "window");
        widget->payload.window.label.content = createstring(widget->payload.window.label.content, "undefined");

    }

    widget = parser_createwidget(ALFI_WIDGET_VSTACK, "window");

    if (widget)
    {

        widget->header.id.name = createstring(widget->header.id.name, "main");
        widget->payload.vstack.halign.direction = ALFI_HALIGN_LEFT;
        widget->payload.vstack.valign.direction = ALFI_VALIGN_TOP;

    }

}

int main(int argc, char **argv)
{

    char data[4096];
    int count;

    pool_setup();
    parser_init(&parser, parser_fail, pool_findbyname, parser_createwidget, parser_destroywidget, pool_allocate);
    loadbase();

    while ((count = read(0, data, 4096)))
        parser_parse(&parser, "main", count, data);

    return 0;

}

