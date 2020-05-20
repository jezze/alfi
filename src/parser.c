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
#define COMMANDS                        5
#define ATTRIBUTES                      11
#define WIDGETS                         15
#define ICONS                           2
#define MODES                           3
#define TARGETS                         2
#define TYPES                           2

struct tokword
{

    unsigned int token;
    char *word;

};

static const struct tokword t_command[] = {
    {COMMAND_NONE, ""},
    {COMMAND_COMMENT, "#"},
    {COMMAND_DELETE, "-"},
    {COMMAND_INSERT, "+"},
    {COMMAND_UPDATE, "="}
};

static const struct tokword t_attribute[] = {
    {ATTRIBUTE_DATA, "data"},
    {ATTRIBUTE_GRID, "grid"},
    {ATTRIBUTE_ICON, "icon"},
    {ATTRIBUTE_ID, "id"},
    {ATTRIBUTE_IN, "in"},
    {ATTRIBUTE_LABEL, "label"},
    {ATTRIBUTE_LINK, "link"},
    {ATTRIBUTE_MODE, "mode"},
    {ATTRIBUTE_RANGE, "range"},
    {ATTRIBUTE_TARGET, "target"},
    {ATTRIBUTE_TYPE, "type"}
};

static const struct tokword t_attribute_icon[] = {
    {ATTRIBUTE_ICON_BURGER, "burger"},
    {ATTRIBUTE_ICON_SEARCH, "search"}
};

static const struct tokword t_attribute_mode[] = {
    {ATTRIBUTE_MODE_OFF, "off"},
    {ATTRIBUTE_MODE_ON, "on"},
    {ATTRIBUTE_MODE_DISABLED, "disabled"}
};

static const struct tokword t_attribute_target[] = {
    {ATTRIBUTE_TARGET_SELF, "self"},
    {ATTRIBUTE_TARGET_BLANK, "blank"}
};

static const struct tokword t_attribute_type[] = {
    {ATTRIBUTE_TYPE_REGULAR, "regular"},
    {ATTRIBUTE_TYPE_PASSWORD, "password"}
};

static const struct tokword t_widget[] = {
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

static unsigned int gettoken(const struct tokword *items, unsigned int nitems, unsigned int count, char *word)
{

    unsigned int i;

    for (i = 0; i < nitems; i++)
    {

        if (!memcmp(word, items[i].word, count + 1))
            return items[i].token;

    }

    return 0;

}

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

static struct widget *parsewidget(struct parser *parser)
{

    char word[32];
    unsigned int count = readword(parser, word, 32);

    return (count) ? parser->find(word) : 0;

}

static unsigned int parsetoken(struct parser *parser, const struct tokword *items, unsigned int nitems)
{

    char word[32];
    unsigned int count = readword(parser, word, 32);

    return (count) ? gettoken(items, nitems, count, word) : 0;

}

static unsigned int parseuint(struct parser *parser, unsigned int base)
{

    char word[32];
    unsigned int count = readword(parser, word, 32);
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

static unsigned int getcommand(struct parser *parser)
{

    return parsetoken(parser, t_command, COMMANDS);

}

static unsigned int getattribute(struct parser *parser)
{

    return parsetoken(parser, t_attribute, ATTRIBUTES);

}

static unsigned int getwidget(struct parser *parser)
{

    return parsetoken(parser, t_widget, WIDGETS);

}

static void parse_attribute_data(struct parser *parser, struct attribute_data *attribute)
{

    char content[RESOURCE_PAGESIZE];

    readword(parser, content, RESOURCE_PAGESIZE);
    attribute_data_create(attribute, content);

}

static void parse_attribute_grid(struct parser *parser, struct attribute_grid *attribute)
{

    char format[128];

    readword(parser, format, 128);
    attribute_grid_create(attribute, format);

}

static void parse_attribute_icon(struct parser *parser, struct attribute_icon *attribute)
{

    unsigned int type = parsetoken(parser, t_attribute_icon, ICONS);

    attribute_icon_create(attribute, type);

}

static void parse_attribute_id(struct parser *parser, struct attribute_id *attribute)
{

    char name[64];

    readword(parser, name, 64);
    attribute_id_create(attribute, name);

}

static void parse_attribute_in(struct parser *parser, struct attribute_in *attribute)
{

    char name[64];

    readword(parser, name, 64);
    attribute_in_create(attribute, name);

}

static void parse_attribute_label(struct parser *parser, struct attribute_label *attribute)
{

    char content[RESOURCE_PAGESIZE];

    readword(parser, content, RESOURCE_PAGESIZE);
    attribute_label_create(attribute, content);

}

static void parse_attribute_link(struct parser *parser, struct attribute_link *attribute)
{

    char url[1024];
    char mime[128];

    readword(parser, url, 1024);
    readword(parser, mime, 128);
    attribute_link_create(attribute, url, mime);

}

static void parse_attribute_mode(struct parser *parser, struct attribute_mode *attribute)
{

    unsigned int type = parsetoken(parser, t_attribute_mode, MODES);

    attribute_mode_create(attribute, type);

}

static void parse_attribute_range(struct parser *parser, struct attribute_range *attribute)
{

    unsigned int min = parseuint(parser, 10);
    unsigned int max = parseuint(parser, 10);

    attribute_range_create(attribute, min, max);

}

static void parse_attribute_target(struct parser *parser, struct attribute_target *attribute)
{

    unsigned int type = parsetoken(parser, t_attribute_target, TARGETS);

    attribute_target_create(attribute, type);

}

static void parse_attribute_type(struct parser *parser, struct attribute_type *attribute)
{

    unsigned int type = parsetoken(parser, t_attribute_type, TYPES);

    attribute_type_create(attribute, type);

}

static void parse_payload_anchor(struct parser *parser, struct widget_header *header, struct widget_payload_anchor *payload)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        case ATTRIBUTE_TARGET:
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

        case ATTRIBUTE_ICON:
            parse_attribute_icon(parser, &payload->icon);

            break;

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_LINK:
            parse_attribute_link(parser, &payload->link);

            break;

        case ATTRIBUTE_MODE:
            parse_attribute_mode(parser, &payload->mode);

            break;

        case ATTRIBUTE_TARGET:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_MODE:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_LINK:
            parse_attribute_link(parser, &payload->link);

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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
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

        case ATTRIBUTE_DATA:
            parse_attribute_data(parser, &payload->data);

            break;

        case ATTRIBUTE_ICON:
            parse_attribute_icon(parser, &payload->icon);

            break;

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_TYPE:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LINK:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
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

        case ATTRIBUTE_DATA:
            parse_attribute_data(parser, &payload->data);

            break;

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_RANGE:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
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

        case ATTRIBUTE_GRID:
            parse_attribute_grid(parser, &payload->grid);

            break;

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_LINK:
            parse_attribute_link(parser, &payload->link);

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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_IN:
            parse_attribute_in(parser, &header->in);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        case ATTRIBUTE_MODE:
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

        case ATTRIBUTE_ID:
            parse_attribute_id(parser, &header->id);

            break;

        case ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &payload->label);

            break;

        default:
            parser->fail();

            break;

        }

    }

}

static void parse_payload(struct parser *parser, struct widget *widget)
{

    switch (widget->header.type)
    {

    case WIDGET_TYPE_ANCHOR:
        parse_payload_anchor(parser, &widget->header, &widget->payload.anchor);

        break;

    case WIDGET_TYPE_BUTTON:
        parse_payload_button(parser, &widget->header, &widget->payload.button);

        break;

    case WIDGET_TYPE_CHOICE:
        parse_payload_choice(parser, &widget->header, &widget->payload.choice);

        break;

    case WIDGET_TYPE_CODE:
        parse_payload_code(parser, &widget->header, &widget->payload.code);

        break;

    case WIDGET_TYPE_DIVIDER:
        parse_payload_divider(parser, &widget->header, &widget->payload.divider);

        break;

    case WIDGET_TYPE_FIELD:
        parse_payload_field(parser, &widget->header, &widget->payload.field);

        break;

    case WIDGET_TYPE_HEADER:
        parse_payload_header(parser, &widget->header, &widget->payload.header);

        break;

    case WIDGET_TYPE_IMAGE:
        parse_payload_image(parser, &widget->header, &widget->payload.image);

        break;

    case WIDGET_TYPE_LIST:
        parse_payload_list(parser, &widget->header, &widget->payload.list);

        break;

    case WIDGET_TYPE_SELECT:
        parse_payload_select(parser, &widget->header, &widget->payload.select);

        break;

    case WIDGET_TYPE_SUBHEADER:
        parse_payload_subheader(parser, &widget->header, &widget->payload.subheader);

        break;

    case WIDGET_TYPE_TABLE:
        parse_payload_table(parser, &widget->header, &widget->payload.table);

        break;

    case WIDGET_TYPE_TEXT:
        parse_payload_text(parser, &widget->header, &widget->payload.text);

        break;

    case WIDGET_TYPE_TOGGLE:
        parse_payload_toggle(parser, &widget->header, &widget->payload.toggle);

        break;

    case WIDGET_TYPE_WINDOW:
        parse_payload_window(parser, &widget->header, &widget->payload.window);

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

    parse_payload(parser, widget);

}

static void parse_command_update(struct parser *parser)
{

    struct widget *widget = parsewidget(parser);

    if (!widget)
        parser->fail();

    parse_payload(parser, widget);

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

void parser_init(struct parser *parser, void (*fail)(void), struct widget *(*find)(char *name), struct widget *(*create)(unsigned int type, char *id, char *in), struct widget *(*destroy)(struct widget *widget), void (*clear)(struct widget *widget))
{

    parser->fail = fail;
    parser->find = find;
    parser->create = create;
    parser->destroy = destroy;
    parser->clear = clear;

}

