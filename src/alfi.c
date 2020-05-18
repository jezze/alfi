#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "attributes.h"
#include "widgets.h"
#include "entity.h"
#include "parser.h"
#include "pool.h"

static struct parser parser;

static struct widget *parser_create(unsigned int type, char *id, char *in)
{

    struct widget *widget = pool_widget_create();

    widget->header.type = type;
    widget->header.id.name = pool_string_create(ALFI_ATTRIBUTE_ID, widget->header.id.name, id);
    widget->header.in.name = pool_string_create(ALFI_ATTRIBUTE_IN, widget->header.in.name, in);

    return widget;

}

static struct widget *parser_destroy(struct widget *widget)
{

    widget->header.id.name = pool_string_destroy(ALFI_ATTRIBUTE_ID, widget->header.id.name);
    widget->header.in.name = pool_string_destroy(ALFI_ATTRIBUTE_IN, widget->header.in.name);

    return pool_widget_destroy(widget);

}

static void parser_clear(struct widget *widget)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        parser_clear(child);

        child = parser_destroy(child);

    }

}

static void parser_fail(void)
{

    printf("Parsing failed on line %u\n", parser.expr.line + 1);
    exit(EXIT_FAILURE);

}

int main(int argc, char **argv)
{

    char data[RESOURCE_PAGESIZE];
    int count;

    pool_setup();
    parser_init(&parser, parser_fail, pool_widget_find, parser_create, parser_destroy, parser_clear, pool_allocate);
    parser_create(ALFI_WIDGET_WINDOW, "window", "");
    parser_create(ALFI_WIDGET_TABLE, "main", "window");

    while ((count = read(0, data, RESOURCE_PAGESIZE)))
        parser_parse(&parser, "main", count, data);

    return 0;

}

