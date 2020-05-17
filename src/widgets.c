#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "widgets.h"
#include "pool.h"
#include "render.h"

struct call
{

    unsigned int flags;
    void (*create)(struct widget *widget);
    void (*destroy)(struct widget *widget);
    unsigned int (*setstate)(struct widget *widget, unsigned int state);

};

static struct call calls[64];

static void anchor_create(struct widget *widget)
{

    struct payload_anchor *payload = &widget->payload.anchor;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

static void anchor_destroy(struct widget *widget)
{

    struct payload_anchor *payload = &widget->payload.anchor;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

static unsigned int anchor_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void button_create(struct widget *widget)
{

    struct payload_button *payload = &widget->payload.button;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

static void button_destroy(struct widget *widget)
{

    struct payload_button *payload = &widget->payload.button;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

static unsigned int button_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void choice_create(struct widget *widget)
{

    struct payload_choice *payload = &widget->payload.choice;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void choice_destroy(struct widget *widget)
{

    struct payload_choice *payload = &widget->payload.choice;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int choice_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void code_create(struct widget *widget)
{

    struct payload_code *payload = &widget->payload.code;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void code_destroy(struct widget *widget)
{

    struct payload_code *payload = &widget->payload.code;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int code_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void divider_create(struct widget *widget)
{

    struct payload_divider *payload = &widget->payload.divider;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void divider_destroy(struct widget *widget)
{

    struct payload_divider *payload = &widget->payload.divider;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int divider_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void field_create(struct widget *widget)
{

    struct payload_field *payload = &widget->payload.field;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void field_destroy(struct widget *widget)
{

    struct payload_field *payload = &widget->payload.field;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

static unsigned int field_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void header_create(struct widget *widget)
{

    struct payload_header *payload = &widget->payload.header;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void header_destroy(struct widget *widget)
{

    struct payload_header *payload = &widget->payload.header;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int header_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void image_create(struct widget *widget)
{

}

static void image_destroy(struct widget *widget)
{

}

static unsigned int image_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void list_create(struct widget *widget)
{

}

static void list_destroy(struct widget *widget)
{

}

static unsigned int list_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void select_create(struct widget *widget)
{

    struct payload_select *payload = &widget->payload.select;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void select_destroy(struct widget *widget)
{

    struct payload_select *payload = &widget->payload.select;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

static unsigned int select_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void subheader_create(struct widget *widget)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void subheader_destroy(struct widget *widget)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int subheader_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void table_create(struct widget *widget)
{

    struct payload_table *payload = &widget->payload.table;

    payload->grid.format = pool_string_create(ALFI_ATTRIBUTE_GRID, payload->grid.format, "12LT");

}

static void table_destroy(struct widget *widget)
{

    struct payload_table *payload = &widget->payload.table;

    payload->grid.format = pool_string_destroy(ALFI_ATTRIBUTE_GRID, payload->grid.format);

}

static unsigned int table_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void text_create(struct widget *widget)
{

    struct payload_text *payload = &widget->payload.text;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void text_destroy(struct widget *widget)
{

    struct payload_text *payload = &widget->payload.text;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int text_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static void toggle_create(struct widget *widget)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void toggle_destroy(struct widget *widget)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int toggle_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static void window_create(struct widget *widget)
{

    struct payload_window *payload = &widget->payload.window;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");

}

static void window_destroy(struct widget *widget)
{

    struct payload_window *payload = &widget->payload.window;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static unsigned int window_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

unsigned int widgets_checkflag(struct widget *widget, unsigned int flag)
{

    return calls[widget->header.type].flags & flag;

}

void widgets_createheader(struct widget *widget, unsigned int type, char *id, char *in)
{

    memset(&widget->header, 0, sizeof (struct header));

    widget->header.type = type;
    widget->header.id.name = pool_string_create(ALFI_ATTRIBUTE_ID, widget->header.id.name, id);
    widget->header.in.name = pool_string_create(ALFI_ATTRIBUTE_IN, widget->header.in.name, in);

}

void widgets_destroyheader(struct widget *widget)
{

    widget->header.id.name = pool_string_destroy(ALFI_ATTRIBUTE_ID, widget->header.id.name);
    widget->header.in.name = pool_string_destroy(ALFI_ATTRIBUTE_IN, widget->header.in.name);

}

void widgets_createpayload(struct widget *widget)
{

    memset(&widget->payload, 0, sizeof (union payload));

    calls[widget->header.type].create(widget);

}

void widgets_destroypayload(struct widget *widget)
{

    calls[widget->header.type].destroy(widget);

}

void widgets_setstate(struct widget *widget, unsigned int state)
{

    widget->header.state = calls[widget->header.type].setstate(widget, state);

}

static void setcallback(unsigned int type, unsigned int flags, void (*create)(struct widget *widget), void (*destroy)(struct widget *widget), unsigned int (*setstate)(struct widget *widget, unsigned int state))
{

    calls[type].flags = flags;
    calls[type].create = create;
    calls[type].destroy = destroy;
    calls[type].setstate = setstate;

}

void widgets_setup(void)
{

    setcallback(ALFI_WIDGET_ANCHOR, ALFI_FLAG_NONE, anchor_create, anchor_destroy, anchor_setstate);
    setcallback(ALFI_WIDGET_BUTTON, ALFI_FLAG_FOCUSABLE, button_create, button_destroy, button_setstate);
    setcallback(ALFI_WIDGET_CHOICE, ALFI_FLAG_NONE, choice_create, choice_destroy, choice_setstate);
    setcallback(ALFI_WIDGET_CODE, ALFI_FLAG_NONE, code_create, code_destroy, code_setstate);
    setcallback(ALFI_WIDGET_DIVIDER, ALFI_FLAG_NONE, divider_create, divider_destroy, divider_setstate);
    setcallback(ALFI_WIDGET_FIELD, ALFI_FLAG_FOCUSABLE, field_create, field_destroy, field_setstate);
    setcallback(ALFI_WIDGET_HEADER, ALFI_FLAG_NONE, header_create, header_destroy, header_setstate);
    setcallback(ALFI_WIDGET_IMAGE, ALFI_FLAG_NONE, image_create, image_destroy, image_setstate);
    setcallback(ALFI_WIDGET_LIST, ALFI_FLAG_NONE, list_create, list_destroy, list_setstate);
    setcallback(ALFI_WIDGET_SELECT, ALFI_FLAG_FOCUSABLE, select_create, select_destroy, select_setstate);
    setcallback(ALFI_WIDGET_SUBHEADER, ALFI_FLAG_NONE, subheader_create, subheader_destroy, subheader_setstate);
    setcallback(ALFI_WIDGET_TABLE, ALFI_FLAG_NONE, table_create, table_destroy, table_setstate);
    setcallback(ALFI_WIDGET_TEXT, ALFI_FLAG_NONE, text_create, text_destroy, text_setstate);
    setcallback(ALFI_WIDGET_TOGGLE, ALFI_FLAG_FOCUSABLE, toggle_create, toggle_destroy, toggle_setstate);
    setcallback(ALFI_WIDGET_WINDOW, ALFI_FLAG_NONE, window_create, window_destroy, window_setstate);

}

