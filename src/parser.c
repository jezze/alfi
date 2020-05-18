#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "attributes.h"
#include "widgets.h"
#include "entity.h"
#include "parser.h"

#define COMMAND_NONE                    0
#define COMMAND_COMMENT                 1
#define COMMAND_DELETE                  2
#define COMMAND_INSERT                  3
#define COMMAND_UPDATE                  4

struct tokword
{

    unsigned int token;
    char *word;

};

static unsigned int readword(struct parser *parser, char *result, unsigned int count)
{

    unsigned int i = 0;

    for (; parser->expr.offset < parser->expr.count; parser->expr.offset++)
    {

        char c = parser->expr.data[parser->expr.offset];

        switch (c)
        {

        case '\0':
        case '\n':
            parser->expr.escaped = 0;
            parser->expr.line++;
            parser->expr.linebreak = 1;
            parser->expr.offset++;
            result[i] = '\0';

            return i;

        }

        if (parser->expr.escaped)
        {

            parser->expr.escaped = 0;

            switch (c)
            {

            case 'n':
                result[i] = '\n';
                i++;

                break;

            default:
                result[i] = c;
                i++;

                break;

            }

        }

        else
        {

            switch (c)
            {

            case '\\':
                parser->expr.escaped = 1;

                break;

            case ' ':
                if (parser->expr.inside)
                {

                    result[i] = c;
                    i++;

                }

                else
                {

                    result[i] = '\0';

                    if (i == 0)
                        continue;

                    parser->expr.offset++;

                    return i;

                }

                break;

            case '"':
                parser->expr.inside = !parser->expr.inside;

                break;

            default:
                result[i] = c;
                i++;

                break;

            }

        }

    }

    return 0;

}

static void parseskip(struct parser *parser)
{

    char word[RESOURCE_PAGESIZE];

    readword(parser, word, RESOURCE_PAGESIZE);

}

struct widget *parsewidget(struct parser *parser)
{

    char word[RESOURCE_PAGESIZE];
    unsigned int count = readword(parser, word, RESOURCE_PAGESIZE);

    return (count) ? parser->find(word) : 0;

}

static unsigned int parsetoken(struct parser *parser, const struct tokword *items, unsigned int nitems)
{

    char word[RESOURCE_PAGESIZE];
    unsigned int count = readword(parser, word, RESOURCE_PAGESIZE);

    if (count)
    {

        unsigned int i;

        for (i = 0; i < nitems; i++)
        {

            if (!memcmp(word, items[i].word, count + 1))
                return items[i].token;

        }

    }

    return 0;

}

static unsigned int parseuint(struct parser *parser, unsigned int base)
{

    char word[RESOURCE_PAGESIZE];
    unsigned int count = readword(parser, word, RESOURCE_PAGESIZE);
    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        switch (word[i])
        {

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            value = value * base + (word[i] - '0');

            break;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            value = value * base + (word[i] - 'A' + 10);

            break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            value = value * base + (word[i] - 'a' + 10);

            break;

        default:
            parser->fail();

            break;

        }

    }

    return value;

}

static char *parsestring(struct parser *parser, unsigned int type, char *string)
{

    char word[RESOURCE_PAGESIZE];
    unsigned int count = readword(parser, word, RESOURCE_PAGESIZE);

    return (count) ? parser->allocate(type, string, count + 1, count + 1, word) : 0;

}

static unsigned int getattribute(struct parser *parser)
{

    static const struct tokword items[] = {
        {ATTRIBUTE_TYPE_NONE, ""},
        {ATTRIBUTE_TYPE_DATA, "data"},
        {ATTRIBUTE_TYPE_GRID, "grid"},
        {ATTRIBUTE_TYPE_ICON, "icon"},
        {ATTRIBUTE_TYPE_ID, "id"},
        {ATTRIBUTE_TYPE_IN, "in"},
        {ATTRIBUTE_TYPE_LABEL, "label"},
        {ATTRIBUTE_TYPE_LINK, "link"},
        {ATTRIBUTE_TYPE_MODE, "mode"},
        {ATTRIBUTE_TYPE_RANGE, "range"},
        {ATTRIBUTE_TYPE_TARGET, "target"},
        {ATTRIBUTE_TYPE_TYPE, "type"}
    };

    return parsetoken(parser, items, 12);

}

static unsigned int getcommand(struct parser *parser)
{

    static const struct tokword items[] = {
        {COMMAND_NONE, ""},
        {COMMAND_COMMENT, "#"},
        {COMMAND_DELETE, "-"},
        {COMMAND_INSERT, "+"},
        {COMMAND_UPDATE, "="}
    };

    return parsetoken(parser, items, 5);

}

static unsigned int geticon(struct parser *parser)
{

    static const struct tokword items[] = {
        {ATTRIBUTE_ICON_BURGER, "burger"},
        {ATTRIBUTE_ICON_SEARCH, "search"}
    };

    return parsetoken(parser, items, 2);

}

static unsigned int getmode(struct parser *parser)
{

    static const struct tokword items[] = {
        {ATTRIBUTE_MODE_OFF, "off"},
        {ATTRIBUTE_MODE_ON, "on"},
        {ATTRIBUTE_MODE_DISABLED, "disabled"}
    };

    return parsetoken(parser, items, 3);

}

static unsigned int gettarget(struct parser *parser)
{

    static const struct tokword items[] = {
        {ATTRIBUTE_TARGET_SELF, "self"},
        {ATTRIBUTE_TARGET_BLANK, "blank"}
    };

    return parsetoken(parser, items, 2);

}

static unsigned int gettype(struct parser *parser)
{

    static const struct tokword items[] = {
        {ATTRIBUTE_TYPE_REGULAR, "regular"},
        {ATTRIBUTE_TYPE_PASSWORD, "password"}
    };

    return parsetoken(parser, items, 2);

}

static unsigned int getwidget(struct parser *parser)
{

    static const struct tokword items[] = {
        {WIDGET_TYPE_ANCHOR, "anchor"},
        {WIDGET_TYPE_BUTTON, "button"},
        {WIDGET_TYPE_CHOICE, "choice"},
        {WIDGET_TYPE_CODE, "code"},
        {WIDGET_TYPE_DIVIDER, "divider"},
        {WIDGET_TYPE_FIELD, "field"},
        {WIDGET_TYPE_HEADER, "header"},
        {WIDGET_TYPE_IMAGE, "image"},
        {WIDGET_TYPE_LIST, "list"},
        {WIDGET_TYPE_SELECT, "select"},
        {WIDGET_TYPE_SUBHEADER, "subheader"},
        {WIDGET_TYPE_TABLE, "table"},
        {WIDGET_TYPE_TEXT, "text"},
        {WIDGET_TYPE_TOGGLE, "toggle"},
        {WIDGET_TYPE_WINDOW, "window"}
    };

    return parsetoken(parser, items, 15);

}

static void parse_attribute_data(struct parser *parser, struct attribute_data *attribute)
{

    attribute->total = WIDGET_DATASIZE;
    attribute->content = parser->allocate(ATTRIBUTE_TYPE_DATA, attribute->content, attribute->total, 0, 0);
    attribute->offset = readword(parser, attribute->content, attribute->total);

}

static void parse_attribute_grid(struct parser *parser, struct attribute_grid *attribute)
{

    attribute->format = parsestring(parser, ATTRIBUTE_TYPE_GRID, attribute->format);

}

static void parse_attribute_icon(struct parser *parser, struct attribute_icon *attribute)
{

    attribute->type = geticon(parser);

}

static void parse_attribute_id(struct parser *parser, struct attribute_id *attribute)
{

    attribute->name = parsestring(parser, ATTRIBUTE_TYPE_ID, attribute->name);

}

static void parse_attribute_in(struct parser *parser, struct attribute_in *attribute)
{

    attribute->name = parsestring(parser, ATTRIBUTE_TYPE_IN, attribute->name);

}

static void parse_attribute_label(struct parser *parser, struct attribute_label *attribute)
{

    attribute->content = parsestring(parser, ATTRIBUTE_TYPE_LABEL, attribute->content);

}

static void parse_attribute_link(struct parser *parser, struct attribute_link *attribute)
{

    attribute->url = parsestring(parser, ATTRIBUTE_TYPE_LINK, attribute->url);
    attribute->mime = parsestring(parser, ATTRIBUTE_TYPE_LINK, attribute->mime);

}

static void parse_attribute_mode(struct parser *parser, struct attribute_mode *attribute)
{

    attribute->type = getmode(parser);

}

static void parse_attribute_range(struct parser *parser, struct attribute_range *attribute)
{

    attribute->min = parseuint(parser, 10);
    attribute->max = parseuint(parser, 10);

}

static void parse_attribute_target(struct parser *parser, struct attribute_target *attribute)
{

    attribute->type = gettarget(parser);

}

static void parse_attribute_type(struct parser *parser, struct attribute_type *attribute)
{

    attribute->type = gettype(parser);

}

static void parse_payload_anchor(struct parser *parser, struct widget_header *header, struct widget_payload_anchor *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_TYPE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        case ATTRIBUTE_TYPE_TARGET:
            parse_attribute_target(parser, &payload->target);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_button(struct parser *parser, struct widget_header *header, struct widget_payload_button *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ICON:
            parse_attribute_icon(parser, &payload->icon);

            break;

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_TYPE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        case ATTRIBUTE_TYPE_MODE:
            parse_attribute_mode(parser, &payload->mode);

            break;

        case ATTRIBUTE_TYPE_TARGET:
            parse_attribute_target(parser, &payload->target);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_choice(struct parser *parser, struct widget_header *header, struct widget_payload_choice *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_TYPE_MODE:
            parse_attribute_mode(parser, &payload->mode);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_code(struct parser *parser, struct widget_header *header, struct widget_payload_code *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_divider(struct parser *parser, struct widget_header *header, struct widget_payload_divider *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_field(struct parser *parser, struct widget_header *header, struct widget_payload_field *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_DATA:
            parse_attribute_data(parser, &payload->data);

            break;

        case ATTRIBUTE_TYPE_ICON:
            parse_attribute_icon(parser, &payload->icon);

            break;

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_TYPE_TYPE:
            parse_attribute_type(parser, &payload->type);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_header(struct parser *parser, struct widget_header *header, struct widget_payload_header *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_image(struct parser *parser, struct widget_header *header, struct widget_payload_image *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_list(struct parser *parser, struct widget_header *header, struct widget_payload_list *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_select(struct parser *parser, struct widget_header *header, struct widget_payload_select *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_DATA:
            parse_attribute_data(parser, &payload->data);

            break;

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_TYPE_RANGE:
            parse_attribute_range(parser, &payload->range);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_subheader(struct parser *parser, struct widget_header *header, struct widget_payload_subheader *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_table(struct parser *parser, struct widget_header *header, struct widget_payload_table *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_GRID:
            parse_attribute_grid(parser, &payload->grid);

            break;

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_text(struct parser *parser, struct widget_header *header, struct widget_payload_text *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_toggle(struct parser *parser, struct widget_header *header, struct widget_payload_toggle *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_TYPE_MODE:
            parse_attribute_mode(parser, &payload->mode);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_window(struct parser *parser, struct widget_header *header, struct widget_payload_window *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_TYPE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_TYPE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload(struct parser *parser, struct widget_header *header, union payload *payload)
{

    switch (header->type)
    {

    case WIDGET_TYPE_ANCHOR:
        parse_payload_anchor(parser, header, &payload->anchor);

        break;

    case WIDGET_TYPE_BUTTON:
        parse_payload_button(parser, header, &payload->button);

        break;

    case WIDGET_TYPE_CHOICE:
        parse_payload_choice(parser, header, &payload->choice);

        break;

    case WIDGET_TYPE_CODE:
        parse_payload_code(parser, header, &payload->code);

        break;

    case WIDGET_TYPE_DIVIDER:
        parse_payload_divider(parser, header, &payload->divider);

        break;

    case WIDGET_TYPE_FIELD:
        parse_payload_field(parser, header, &payload->field);

        break;

    case WIDGET_TYPE_HEADER:
        parse_payload_header(parser, header, &payload->header);

        break;

    case WIDGET_TYPE_IMAGE:
        parse_payload_image(parser, header, &payload->image);

        break;

    case WIDGET_TYPE_LIST:
        parse_payload_list(parser, header, &payload->list);

        break;

    case WIDGET_TYPE_SELECT:
        parse_payload_select(parser, header, &payload->select);

        break;

    case WIDGET_TYPE_SUBHEADER:
        parse_payload_subheader(parser, header, &payload->subheader);

        break;

    case WIDGET_TYPE_TABLE:
        parse_payload_table(parser, header, &payload->table);

        break;

    case WIDGET_TYPE_TEXT:
        parse_payload_text(parser, header, &payload->text);

        break;

    case WIDGET_TYPE_TOGGLE:
        parse_payload_toggle(parser, header, &payload->toggle);

        break;

    case WIDGET_TYPE_WINDOW:
        parse_payload_window(parser, header, &payload->window);

        break;

    default:
        parser->fail();

        break;

    }

}

static void parse_command_comment(struct parser *parser)
{

    while (!parser->expr.linebreak)
        parseskip(parser);

}

static void parse_command_delete(struct parser *parser)
{

    struct widget *widget = parsewidget(parser);

    if (!widget)
        parser->fail();

    parser->clear(widget);
    parser->destroy(widget);

}

static void parse_command_insert(struct parser *parser, char *in)
{

    struct widget *widget = parser->create(getwidget(parser), "", in);

    if (!widget)
        parser->fail();

    parse_payload(parser, &widget->header, &widget->payload);

}

static void parse_command_update(struct parser *parser)
{

    struct widget *widget = parsewidget(parser);

    if (!widget)
        parser->fail();

    parse_payload(parser, &widget->header, &widget->payload);

}

static void parse(struct parser *parser, char *in)
{

    while (parser->expr.offset < parser->expr.count)
    {

        parser->expr.linebreak = 0;

        switch (getcommand(parser))
        {

        case COMMAND_NONE:
            break;

        case COMMAND_COMMENT:
            parse_command_comment(parser);

            break;

        case COMMAND_DELETE:
            parse_command_delete(parser);

            break;

        case COMMAND_INSERT:
            parse_command_insert(parser, in);

            break;

        case COMMAND_UPDATE:
            parse_command_update(parser);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

void parser_parse(struct parser *parser, char *in, unsigned int count, void *data)
{

    parser->expr.data = data;
    parser->expr.count = count;
    parser->expr.offset = 0;
    parser->expr.line = 0;
    parser->expr.linebreak = 0;
    parser->expr.inside = 0;
    parser->expr.escaped = 0;

    parse(parser, in);

}

void parser_init(struct parser *parser, void (*fail)(void), struct widget *(*find)(char *name), struct widget *(*create)(unsigned int type, char *id, char *in), struct widget *(*destroy)(struct widget *widget), void (*clear)(struct widget *widget), char *(*allocate)(unsigned int type, char *string, unsigned int size, unsigned int count, char *content))
{

    parser->fail = fail;
    parser->find = find;
    parser->create = create;
    parser->destroy = destroy;
    parser->clear = clear;
    parser->allocate = allocate;

}

