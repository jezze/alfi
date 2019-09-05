#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
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
            parser->expr.line++;
            parser->expr.linebreak = 1;
            parser->expr.offset++;
            result[i] = '\0';

            return i;

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

    return 0;

}

static unsigned int parsetoken(struct parser *parser, const struct tokword *items, unsigned int nitems)
{

    char word[4096];
    unsigned int count = readword(parser, word, 4096);
    unsigned int i;

    for (i = 0; i < nitems; i++)
    {

        if (!memcmp(word, items[i].word, count + 1))
            return items[i].token;

    }

    return 0;

}

static unsigned int parseuint(struct parser *parser, unsigned int base)
{

    char word[4096];
    unsigned int count = readword(parser, word, 4096);
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
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

    return value;

}

static char *parsestring(char *string, struct parser *parser)
{

    char word[4096];
    unsigned int count = readword(parser, word, 4096);

    return parser->allocate(string, count + 1, count + 1, word);

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
        {ALFI_ATTRIBUTE_RANGE, "range"},
        {ALFI_ATTRIBUTE_TARGET, "target"},
        {ALFI_ATTRIBUTE_TYPE, "type"},
        {ALFI_ATTRIBUTE_VALIGN, "valign"}
    };

    return parsetoken(parser, items, 13);

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
        {ALFI_WIDGET_SUBHEADER, "subheader"},
        {ALFI_WIDGET_HSTACK, "hstack"},
        {ALFI_WIDGET_IMAGE, "image"},
        {ALFI_WIDGET_LIST, "list"},
        {ALFI_WIDGET_SELECT, "select"},
        {ALFI_WIDGET_TAB, "tab"},
        {ALFI_WIDGET_TABLE, "table"},
        {ALFI_WIDGET_TEXT, "text"},
        {ALFI_WIDGET_VSTACK, "vstack"},
        {ALFI_WIDGET_WINDOW, "window"}
    };

    return parsetoken(parser, items, 16);

}

static void parse_attribute_data(struct parser *parser, struct alfi_attribute_data *data)
{

    data->total = ALFI_DATASIZE;
    data->content = parser->allocate(data->content, data->total, 0, 0);
    data->offset = readword(parser, data->content, data->total);

}

static void parse_attribute_grid(struct parser *parser, struct alfi_attribute_grid *grid)
{

    grid->csize = parseuint(parser, 10);
    grid->rsize = parseuint(parser, 10);
    grid->coffset = parseuint(parser, 10);
    grid->roffset = parseuint(parser, 10);
    grid->clength = parseuint(parser, 10);
    grid->rlength = parseuint(parser, 10);

}

static void parse_attribute_halign(struct parser *parser, struct alfi_attribute_halign *halign)
{

    halign->direction = gethalign(parser);

}

static void parse_attribute_icon(struct parser *parser, struct alfi_attribute_icon *icon)
{

    icon->type = geticon(parser);

}

static void parse_attribute_id(struct parser *parser, struct alfi_attribute_id *id)
{

    id->name = parsestring(id->name, parser);

}

static void parse_attribute_in(struct parser *parser, struct alfi_attribute_in *in)
{

    in->name = parsestring(in->name, parser);

}

static void parse_attribute_label(struct parser *parser, struct alfi_attribute_label *label)
{

    label->content = parsestring(label->content, parser);

}

static void parse_attribute_link(struct parser *parser, struct alfi_attribute_link *link)
{

    link->url = parsestring(link->url, parser);
    link->mime = parsestring(link->mime, parser);

}

static void parse_attribute_range(struct parser *parser, struct alfi_attribute_range *range)
{

    range->min = parseuint(parser, 10);
    range->max = parseuint(parser, 10);

}

static void parse_attribute_target(struct parser *parser, struct alfi_attribute_target *target)
{

    target->type = gettarget(parser);

}

static void parse_attribute_type(struct parser *parser, struct alfi_attribute_type *type)
{

    type->type = gettype(parser);

}

static void parse_attribute_valign(struct parser *parser, struct alfi_attribute_valign *valign)
{

    valign->direction = getvalign(parser);

}

static void parse_widget_button(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_button *button)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ICON:
            parse_attribute_icon(parser, &button->icon);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &button->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_choice(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_choice *choice)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &choice->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_divider(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_divider *divider)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &divider->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_field(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_field *field)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_DATA:
            parse_attribute_data(parser, &field->data);

            break;

        case ALFI_ATTRIBUTE_ICON:
            parse_attribute_icon(parser, &field->icon);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &field->label);

            break;

        case ALFI_ATTRIBUTE_TYPE:
            parse_attribute_type(parser, &widget->data.field.type);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_header(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_header *header)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &header->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_hstack(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_hstack *hstack)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_GRID:
            parse_attribute_grid(parser, &hstack->grid);

            break;

        case ALFI_ATTRIBUTE_HALIGN:
            parse_attribute_halign(parser, &hstack->halign);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_VALIGN:
            parse_attribute_valign(parser, &hstack->valign);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_image(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_image *image)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LINK:
            parse_attribute_link(parser, &image->link);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_anchor(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_anchor *anchor)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &anchor->label);

            break;

        case ALFI_ATTRIBUTE_LINK:
            parse_attribute_link(parser, &anchor->link);

            break;

        case ALFI_ATTRIBUTE_TARGET:
            parse_attribute_target(parser, &anchor->target);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_list(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_list *list)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_select(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_select *select)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_DATA:
            parse_attribute_data(parser, &select->data);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &select->label);

            break;

        case ALFI_ATTRIBUTE_RANGE:
            parse_attribute_range(parser, &select->range);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_subheader(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_subheader *subheader)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &subheader->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_tab(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_tab *tab)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &tab->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_table(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_table *table)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_GRID:
            parse_attribute_grid(parser, &table->grid);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_text(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_text *text)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &text->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_vstack(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_vstack *vstack)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_GRID:
            parse_attribute_grid(parser, &vstack->grid);

            break;

        case ALFI_ATTRIBUTE_HALIGN:
            parse_attribute_halign(parser, &vstack->halign);

            break;

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_IN:
            parse_attribute_in(parser, &widget->in);

            break;

        case ALFI_ATTRIBUTE_VALIGN:
            parse_attribute_valign(parser, &vstack->valign);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_widget_window(struct parser *parser, struct alfi_widget *widget, struct alfi_widget_window *window)
{

    while (!parser->expr.linebreak)
    {

        switch (getattribute(parser))
        {

        case ALFI_ATTRIBUTE_ID:
            parse_attribute_id(parser, &widget->id);

            break;

        case ALFI_ATTRIBUTE_LABEL:
            parse_attribute_label(parser, &window->label);

            break;

        default:
            parser->fail(parser->expr.line + 1);

            break;

        }

    }

}

static void parse_command_comment(struct parser *parser)
{

    char word[4096];

    while (!parser->expr.linebreak)
        readword(parser, word, 4096);

}

static void parse_command_delete(struct parser *parser)
{

    char word[4096];
    struct alfi_widget *widget;

    readword(parser, word, 4096);

    widget = parser->findwidget(word);

    if (!widget)
        parser->fail(parser->expr.line + 1);

    parser->destroywidget(widget);

}

static void parse_command_insert(struct parser *parser, char *in)
{

    struct alfi_widget *widget = parser->createwidget(getwidget(parser), in);

    if (!widget)
        parser->fail(parser->expr.line + 1);

    switch (widget->type)
    {

    case ALFI_WIDGET_ANCHOR:
        parse_widget_anchor(parser, widget, &widget->data.anchor);

        break;

    case ALFI_WIDGET_BUTTON:
        parse_widget_button(parser, widget, &widget->data.button);

        break;

    case ALFI_WIDGET_CHOICE:
        parse_widget_choice(parser, widget, &widget->data.choice);

        break;

    case ALFI_WIDGET_DIVIDER:
        parse_widget_divider(parser, widget, &widget->data.divider);

        break;

    case ALFI_WIDGET_FIELD:
        parse_widget_field(parser, widget, &widget->data.field);

        break;

    case ALFI_WIDGET_HEADER:
        parse_widget_header(parser, widget, &widget->data.header);

        break;

    case ALFI_WIDGET_HSTACK:
        parse_widget_hstack(parser, widget, &widget->data.hstack);

        break;

    case ALFI_WIDGET_IMAGE:
        parse_widget_image(parser, widget, &widget->data.image);

        break;

    case ALFI_WIDGET_LIST:
        parse_widget_list(parser, widget, &widget->data.list);

        break;

    case ALFI_WIDGET_SELECT:
        parse_widget_select(parser, widget, &widget->data.select);

        break;

    case ALFI_WIDGET_SUBHEADER:
        parse_widget_subheader(parser, widget, &widget->data.subheader);

        break;

    case ALFI_WIDGET_TAB:
        parse_widget_tab(parser, widget, &widget->data.tab);

        break;

    case ALFI_WIDGET_TABLE:
        parse_widget_table(parser, widget, &widget->data.table);

        break;

    case ALFI_WIDGET_TEXT:
        parse_widget_text(parser, widget, &widget->data.text);

        break;

    case ALFI_WIDGET_VSTACK:
        parse_widget_vstack(parser, widget, &widget->data.vstack);

        break;

    case ALFI_WIDGET_WINDOW:
        parse_widget_window(parser, widget, &widget->data.window);

        break;

    default:
        parser->fail(parser->expr.line + 1);

        break;

    }

}

static void parse_command_update(struct parser *parser)
{

    char word[4096];
    struct alfi_widget *widget;

    readword(parser, word, 4096);

    widget = parser->findwidget(word);

    if (!widget)
        parser->fail(parser->expr.line + 1);

    switch (widget->type)
    {

    case ALFI_WIDGET_ANCHOR:
        parse_widget_anchor(parser, widget, &widget->data.anchor);

        break;

    case ALFI_WIDGET_BUTTON:
        parse_widget_button(parser, widget, &widget->data.button);

        break;

    case ALFI_WIDGET_CHOICE:
        parse_widget_choice(parser, widget, &widget->data.choice);

        break;

    case ALFI_WIDGET_DIVIDER:
        parse_widget_divider(parser, widget, &widget->data.divider);

        break;

    case ALFI_WIDGET_FIELD:
        parse_widget_field(parser, widget, &widget->data.field);

        break;

    case ALFI_WIDGET_HEADER:
        parse_widget_header(parser, widget, &widget->data.header);

        break;

    case ALFI_WIDGET_HSTACK:
        parse_widget_hstack(parser, widget, &widget->data.hstack);

        break;

    case ALFI_WIDGET_IMAGE:
        parse_widget_image(parser, widget, &widget->data.image);

        break;

    case ALFI_WIDGET_LIST:
        parse_widget_list(parser, widget, &widget->data.list);

        break;

    case ALFI_WIDGET_SELECT:
        parse_widget_select(parser, widget, &widget->data.select);

        break;

    case ALFI_WIDGET_SUBHEADER:
        parse_widget_subheader(parser, widget, &widget->data.subheader);

        break;

    case ALFI_WIDGET_TAB:
        parse_widget_tab(parser, widget, &widget->data.tab);

        break;

    case ALFI_WIDGET_TABLE:
        parse_widget_table(parser, widget, &widget->data.table);

        break;

    case ALFI_WIDGET_TEXT:
        parse_widget_text(parser, widget, &widget->data.text);

        break;

    case ALFI_WIDGET_VSTACK:
        parse_widget_vstack(parser, widget, &widget->data.vstack);

        break;

    case ALFI_WIDGET_WINDOW:
        parse_widget_window(parser, widget, &widget->data.window);

        break;

    default:
        parser->fail(parser->expr.line + 1);

        break;

    }

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
            parser->fail(parser->expr.line + 1);

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

    parse(parser, in);

}

void parser_init(struct parser *parser, void (*fail)(unsigned int line), struct alfi_widget *(*findwidget)(char *name), struct alfi_widget *(*createwidget)(unsigned int type, char *in), struct alfi_widget *(*destroywidget)(struct alfi_widget *widget), char *(*allocate)(char *string, unsigned int size, unsigned int count, char *content), char *(*createstring)(char *string, char *content), char *(*destroystring)(char *string))
{

    parser->fail = fail;
    parser->findwidget = findwidget;
    parser->createwidget = createwidget;
    parser->destroywidget = destroywidget;
    parser->allocate = allocate;
    parser->createstring = createstring;
    parser->destroystring = destroystring;

}

