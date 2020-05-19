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

    memset(&widget->header, 0, sizeof (struct widget_header));
    widget_header_create(&widget->header, type, id, in);

    return widget;

}

static struct widget *parser_destroy(struct widget *widget)
{

    widget_header_destroy(&widget->header);

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
    parser_init(&parser, parser_fail, pool_widget_find, parser_create, parser_destroy, parser_clear);
    parser_create(WIDGET_TYPE_WINDOW, "window", "");
    parser_create(WIDGET_TYPE_TABLE, "main", "window");

    while ((count = read(0, data, RESOURCE_PAGESIZE)))
        parser_parse(&parser, "main", count, data);

    return 0;

}

