#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "widgets.h"
#include "parser.h"

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
        {ALFI_ATTRIBUTE_NONE, ""},
        {ALFI_ATTRIBUTE_DATA, "data"},
        {ALFI_ATTRIBUTE_GRID, "grid"},
        {ALFI_ATTRIBUTE_HALIGN, "halign"},
        {ALFI_ATTRIBUTE_ICON, "icon"},
        {ALFI_ATTRIBUTE_ID, "id"},
        {ALFI_ATTRIBUTE_IN, "in"},
        {ALFI_ATTRIBUTE_LABEL, "label"},
        {ALFI_ATTRIBUTE_LINK, "link"},
        {ALFI_ATTRIBUTE_MODE, "mode"},
        {ALFI_ATTRIBUTE_RANGE, "range"},
        {ALFI_ATTRIBUTE_TARGET, "target"},
        {ALFI_ATTRIBUTE_TYPE, "type"},
        {ALFI_ATTRIBUTE_VALIGN, "valign"}
    };

    return parsetoken(parser, items, 14);

}

static unsigned int getcommand(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_COMMAND_NONE, ""},
        {ALFI_COMMAND_COMMENT, "#"},
        {ALFI_COMMAND_DELETE, "-"},
        {ALFI_COMMAND_INSERT, "+"},
        {ALFI_COMMAND_UPDATE, "="}
    };

    return parsetoken(parser, items, 5);

}

static unsigned int gethalign(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_HALIGN_LEFT, "left"},
        {ALFI_HALIGN_CENTER, "center"},
        {ALFI_HALIGN_RIGHT, "right"}
    };

    return parsetoken(parser, items, 3);

}

static unsigned int geticon(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_ICON_BURGER, "burger"},
        {ALFI_ICON_SEARCH, "search"}
    };

    return parsetoken(parser, items, 2);

}

static unsigned int getmode(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_MODE_OFF, "off"},
        {ALFI_MODE_ON, "on"},
        {ALFI_MODE_DISABLED, "disabled"}
    };

    return parsetoken(parser, items, 3);

}

static unsigned int gettarget(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_TARGET_SELF, "self"},
        {ALFI_TARGET_BLANK, "blank"}
    };

    return parsetoken(parser, items, 2);

}

static unsigned int gettype(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_TYPE_REGULAR, "regular"},
        {ALFI_TYPE_PASSWORD, "password"}
    };

    return parsetoken(parser, items, 2);

}

static unsigned int getvalign(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_VALIGN_TOP, "top"},
        {ALFI_VALIGN_MIDDLE, "middle"},
        {ALFI_VALIGN_BOTTOM, "bottom"}
    };

    return parsetoken(parser, items, 3);

}

static unsigned int getwidget(struct parser *parser)
{

    static const struct tokword items[] = {
        {ALFI_WIDGET_ANCHOR, "anchor"},
        {ALFI_WIDGET_BUTTON, "button"},
        {ALFI_WIDGET_CHOICE, "choice"},
        {ALFI_WIDGET_DIVIDER, "divider"},
        {ALFI_WIDGET_FIELD, "field"},
        {ALFI_WIDGET_HEADER, "header"},
        {ALFI_WIDGET_IMAGE, "image"},
        {ALFI_WIDGET_LIST, "list"},
        {ALFI_WIDGET_SELECT, "select"},
        {ALFI_WIDGET_STACK, "stack"},
        {ALFI_WIDGET_SUBHEADER, "subheader"},
        {ALFI_WIDGET_TABLE, "table"},
        {ALFI_WIDGET_TEXT, "text"},
        {ALFI_WIDGET_TOGGLE, "toggle"},
        {ALFI_WIDGET_WINDOW, "window"}
    };

    return parsetoken(parser, items, 15);

}

static void parse_attribute_data(struct parser *parser, struct attribute_data *attribute)
{

    attribute->total = ALFI_DATASIZE;
    attribute->content = parser->allocate(ALFI_ATTRIBUTE_DATA, attribute->content, attribute->total, 0, 0);
    attribute->offset = readword(parser, attribute->content, attribute->total);

}

static void parse_attribute_grid(struct parser *parser, struct attribute_grid *attribute)
{

    attribute->csize = parseuint(parser, 10);

}

static void parse_attribute_halign(struct parser *parser, struct attribute_halign *attribute)
{

    attribute->direction = gethalign(parser);

}

static void parse_attribute_icon(struct parser *parser, struct attribute_icon *attribute)
{

    attribute->type = geticon(parser);

}

static void parse_attribute_id(struct parser *parser, struct attribute_id *attribute)
{

    attribute->name = parsestring(parser, ALFI_ATTRIBUTE_ID, attribute->name);

}

static void parse_attribute_in(struct parser *parser, struct attribute_in *attribute)
{

    attribute->name = parsestring(parser, ALFI_ATTRIBUTE_IN, attribute->name);

}

static void parse_attribute_label(struct parser *parser, struct attribute_label *attribute)
{

    attribute->content = parsestring(parser, ALFI_ATTRIBUTE_LABEL, attribute->content);

}

static void parse_attribute_link(struct parser *parser, struct attribute_link *attribute)
{

    attribute->url = parsestring(parser, ALFI_ATTRIBUTE_LINK, attribute->url);
    attribute->mime = parsestring(parser, ALFI_ATTRIBUTE_LINK, attribute->mime);

}

static void parse_attribute_mode(struct parser *parser, struct attribute_mode *attribute)
{

    attribute->mode = getmode(parser);

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

static void parse_attribute_valign(struct parser *parser, struct attribute_valign *attribute)
{

    attribute->direction = getvalign(parser);

}

static void parse_payload_anchor(struct parser *parser, struct header *header, struct payload_anchor *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ALFI_ATTRIBUTE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        case ALFI_ATTRIBUTE_TARGET:
            parse_attribute_target(parser, &payload->target);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_button(struct parser *parser, struct header *header, struct payload_button *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ICON:
            parse_attribute_icon(parser, &payload->icon);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ALFI_ATTRIBUTE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        case ALFI_ATTRIBUTE_MODE:
            parse_attribute_mode(parser, &payload->mode);

            break;

        case ALFI_ATTRIBUTE_TARGET:
            parse_attribute_target(parser, &payload->target);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_choice(struct parser *parser, struct header *header, struct payload_choice *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ALFI_ATTRIBUTE_MODE:
            parse_attribute_mode(parser, &payload->mode);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_divider(struct parser *parser, struct header *header, struct payload_divider *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_field(struct parser *parser, struct header *header, struct payload_field *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_DATA:
            parse_attribute_data(parser, &payload->data);

            break;

        case ALFI_ATTRIBUTE_ICON:
            parse_attribute_icon(parser, &payload->icon);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ALFI_ATTRIBUTE_TYPE:
            parse_attribute_type(parser, &payload->type);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_header(struct parser *parser, struct header *header, struct payload_header *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_image(struct parser *parser, struct header *header, struct payload_image *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_list(struct parser *parser, struct header *header, struct payload_list *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_select(struct parser *parser, struct header *header, struct payload_select *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_DATA:
            parse_attribute_data(parser, &payload->data);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ALFI_ATTRIBUTE_RANGE:
            parse_attribute_range(parser, &payload->range);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_stack(struct parser *parser, struct header *header, struct payload_stack *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_HALIGN:
            parse_attribute_halign(parser, &payload->halign);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_VALIGN:
            parse_attribute_valign(parser, &payload->valign);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_subheader(struct parser *parser, struct header *header, struct payload_subheader *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_table(struct parser *parser, struct header *header, struct payload_table *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_GRID:
            parse_attribute_grid(parser, &payload->grid);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_text(struct parser *parser, struct header *header, struct payload_text *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_toggle(struct parser *parser, struct header *header, struct payload_toggle *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ALFI_ATTRIBUTE_MODE:
            parse_attribute_mode(parser, &payload->mode);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload_window(struct parser *parser, struct header *header, struct payload_window *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload(struct parser *parser, struct header *header, union payload *payload)
{

    switch (header->type)
    {

    case ALFI_WIDGET_ANCHOR:
        parse_payload_anchor(parser, header, &payload->anchor);

        break;

    case ALFI_WIDGET_BUTTON:
        parse_payload_button(parser, header, &payload->button);

        break;

    case ALFI_WIDGET_CHOICE:
        parse_payload_choice(parser, header, &payload->choice);

        break;

    case ALFI_WIDGET_DIVIDER:
        parse_payload_divider(parser, header, &payload->divider);

        break;

    case ALFI_WIDGET_FIELD:
        parse_payload_field(parser, header, &payload->field);

        break;

    case ALFI_WIDGET_HEADER:
        parse_payload_header(parser, header, &payload->header);

        break;

    case ALFI_WIDGET_IMAGE:
        parse_payload_image(parser, header, &payload->image);

        break;

    case ALFI_WIDGET_LIST:
        parse_payload_list(parser, header, &payload->list);

        break;

    case ALFI_WIDGET_SELECT:
        parse_payload_select(parser, header, &payload->select);

        break;

    case ALFI_WIDGET_STACK:
        parse_payload_stack(parser, header, &payload->stack);

        break;

    case ALFI_WIDGET_SUBHEADER:
        parse_payload_subheader(parser, header, &payload->subheader);

        break;

    case ALFI_WIDGET_TABLE:
        parse_payload_table(parser, header, &payload->table);

        break;

    case ALFI_WIDGET_TEXT:
        parse_payload_text(parser, header, &payload->text);

        break;

    case ALFI_WIDGET_TOGGLE:
        parse_payload_toggle(parser, header, &payload->toggle);

        break;

    case ALFI_WIDGET_WINDOW:
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

        case ALFI_COMMAND_NONE:
            break;

        case ALFI_COMMAND_COMMENT:
            parse_command_comment(parser);

            break;

        case ALFI_COMMAND_DELETE:
            parse_command_delete(parser);

            break;

        case ALFI_COMMAND_INSERT:
            parse_command_insert(parser, in);

            break;

        case ALFI_COMMAND_UPDATE:
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

