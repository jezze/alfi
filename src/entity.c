#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "attributes.h"
#include "widgets.h"
#include "entity.h"
#include "pool.h"
#include "render.h"

static void anchor_create(struct payload_anchor *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

static void anchor_destroy(struct payload_anchor *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

static void button_create(struct payload_button *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

static void button_destroy(struct payload_button *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

static unsigned int button_setstate(struct header *header, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (header->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void choice_create(struct payload_choice *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void choice_destroy(struct payload_choice *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static void code_create(struct payload_code *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void code_destroy(struct payload_code *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static void divider_create(struct payload_divider *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void divider_destroy(struct payload_divider *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static void field_create(struct payload_field *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void field_destroy(struct payload_field *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

static unsigned int field_setstate(struct header *header, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (header->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void header_create(struct payload_header *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void header_destroy(struct payload_header *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static void image_create(struct payload_image *payload)
{

}

static void image_destroy(struct payload_image *payload)
{

}

static void list_create(struct payload_list *payload)
{

}

static void list_destroy(struct payload_list *payload)
{

}

static void select_create(struct payload_select *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void select_destroy(struct payload_select *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

static unsigned int select_setstate(struct header *header, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (header->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void subheader_create(struct payload_subheader *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void subheader_destroy(struct payload_subheader *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static void table_create(struct payload_table *payload)
{

    payload->grid.format = pool_string_create(ALFI_ATTRIBUTE_GRID, payload->grid.format, "12LT");

}

static void table_destroy(struct payload_table *payload)
{

    payload->grid.format = pool_string_destroy(ALFI_ATTRIBUTE_GRID, payload->grid.format);

}

static void text_create(struct payload_text *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void text_destroy(struct payload_text *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static void toggle_create(struct payload_toggle *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void toggle_destroy(struct payload_toggle *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int toggle_setstate(struct header *header, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (header->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void window_create(struct payload_window *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");

}

static void window_destroy(struct payload_window *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

void entity_createheader(struct widget *widget, unsigned int type, char *id, char *in)
{

    memset(&widget->header, 0, sizeof (struct header));

    widget->header.type = type;
    widget->header.id.name = pool_string_create(ALFI_ATTRIBUTE_ID, widget->header.id.name, id);
    widget->header.in.name = pool_string_create(ALFI_ATTRIBUTE_IN, widget->header.in.name, in);

}

void entity_destroyheader(struct widget *widget)
{

    widget->header.id.name = pool_string_destroy(ALFI_ATTRIBUTE_ID, widget->header.id.name);
    widget->header.in.name = pool_string_destroy(ALFI_ATTRIBUTE_IN, widget->header.in.name);

}

static unsigned int getflags(struct widget *widget)
{

    switch (widget->header.type)
    {

    case ALFI_WIDGET_BUTTON:
    case ALFI_WIDGET_FIELD:
    case ALFI_WIDGET_SELECT:
    case ALFI_WIDGET_TOGGLE:
        return ALFI_FLAG_FOCUSABLE;

    }

    return ALFI_FLAG_NONE;

}

unsigned int entity_checkflag(struct widget *widget, unsigned int flag)
{

    return getflags(widget) & flag;

}

void entity_createpayload(struct widget *widget)
{

    memset(&widget->payload, 0, sizeof (union payload));

    switch (widget->header.type)
    {

    case ALFI_WIDGET_ANCHOR:
        anchor_create(&widget->payload.anchor);

        break;

    case ALFI_WIDGET_BUTTON:
        button_create(&widget->payload.button);

        break;

    case ALFI_WIDGET_CHOICE:
        choice_create(&widget->payload.choice);

        break;

    case ALFI_WIDGET_CODE:
        code_create(&widget->payload.code);

        break;

    case ALFI_WIDGET_DIVIDER:
        divider_create(&widget->payload.divider);

        break;

    case ALFI_WIDGET_FIELD:
        field_create(&widget->payload.field);

        break;

    case ALFI_WIDGET_HEADER:
        header_create(&widget->payload.header);

        break;

    case ALFI_WIDGET_IMAGE:
        image_create(&widget->payload.image);

        break;

    case ALFI_WIDGET_LIST:
        list_create(&widget->payload.list);

        break;

    case ALFI_WIDGET_SELECT:
        select_create(&widget->payload.select);

        break;

    case ALFI_WIDGET_SUBHEADER:
        subheader_create(&widget->payload.subheader);

        break;

    case ALFI_WIDGET_TABLE:
        table_create(&widget->payload.table);

        break;

    case ALFI_WIDGET_TEXT:
        text_create(&widget->payload.text);

        break;

    case ALFI_WIDGET_TOGGLE:
        toggle_create(&widget->payload.toggle);

        break;

    case ALFI_WIDGET_WINDOW:
        window_create(&widget->payload.window);

        break;

    }

}

void entity_destroypayload(struct widget *widget)
{

    switch (widget->header.type)
    {

    case ALFI_WIDGET_ANCHOR:
        anchor_destroy(&widget->payload.anchor);

        break;

    case ALFI_WIDGET_BUTTON:
        button_destroy(&widget->payload.button);

        break;

    case ALFI_WIDGET_CHOICE:
        choice_destroy(&widget->payload.choice);

        break;

    case ALFI_WIDGET_CODE:
        code_destroy(&widget->payload.code);

        break;

    case ALFI_WIDGET_DIVIDER:
        divider_destroy(&widget->payload.divider);

        break;

    case ALFI_WIDGET_FIELD:
        field_destroy(&widget->payload.field);

        break;

    case ALFI_WIDGET_HEADER:
        header_destroy(&widget->payload.header);

        break;

    case ALFI_WIDGET_IMAGE:
        image_destroy(&widget->payload.image);

        break;

    case ALFI_WIDGET_LIST:
        list_destroy(&widget->payload.list);

        break;

    case ALFI_WIDGET_SELECT:
        select_destroy(&widget->payload.select);

        break;

    case ALFI_WIDGET_SUBHEADER:
        subheader_destroy(&widget->payload.subheader);

        break;

    case ALFI_WIDGET_TABLE:
        table_destroy(&widget->payload.table);

        break;

    case ALFI_WIDGET_TEXT:
        text_destroy(&widget->payload.text);

        break;

    case ALFI_WIDGET_TOGGLE:
        toggle_destroy(&widget->payload.toggle);

        break;

    case ALFI_WIDGET_WINDOW:
        window_destroy(&widget->payload.window);

        break;

    }

}

void entity_setstate(struct widget *widget, unsigned int state)
{

    switch (widget->header.type)
    {

    case ALFI_WIDGET_BUTTON:
        widget->header.state = button_setstate(&widget->header, state);

        break;

    case ALFI_WIDGET_FIELD:
        widget->header.state = field_setstate(&widget->header, state);

        break;

    case ALFI_WIDGET_SELECT:
        widget->header.state = select_setstate(&widget->header, state);

        break;

    case ALFI_WIDGET_TOGGLE:
        widget->header.state = toggle_setstate(&widget->header, state);

        break;

    default:
        widget->header.state = ALFI_STATE_NORMAL;

        break;

    }

}

