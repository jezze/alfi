#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "attribute.h"
#include "widget.h"
#include "entity.h"
#include "pool.h"
#include "render.h"

static unsigned int getflags(struct widget *widget)
{

    switch (widget->header.type)
    {

    case WIDGET_TYPE_BUTTON:
    case WIDGET_TYPE_FIELD:
    case WIDGET_TYPE_SELECT:
    case WIDGET_TYPE_TOGGLE:
        return ENTITY_FLAG_FOCUSABLE;

    }

    return ENTITY_FLAG_NONE;

}

unsigned int entity_checkflag(struct widget *widget, unsigned int flag)
{

    return getflags(widget) & flag;

}

void entity_create(struct widget *widget, unsigned int type, char *id, char *in)
{

    memset(&widget->header, 0, sizeof (struct widget_header));
    memset(&widget->payload, 0, sizeof (union widget_payload));
    memset(&widget->frame, 0, sizeof (struct frame));
    widget_header_create(&widget->header, type, id, in);

    switch (widget->header.type)
    {

    case WIDGET_TYPE_ANCHOR:
        widget_payload_anchor_create(&widget->payload.anchor);

        break;

    case WIDGET_TYPE_BUTTON:
        widget_payload_button_create(&widget->payload.button);

        break;

    case WIDGET_TYPE_CHOICE:
        widget_payload_choice_create(&widget->payload.choice);

        break;

    case WIDGET_TYPE_CODE:
        widget_payload_code_create(&widget->payload.code);

        break;

    case WIDGET_TYPE_DIVIDER:
        widget_payload_divider_create(&widget->payload.divider);

        break;

    case WIDGET_TYPE_FIELD:
        widget_payload_field_create(&widget->payload.field);

        break;

    case WIDGET_TYPE_HEADER:
        widget_payload_header_create(&widget->payload.header);

        break;

    case WIDGET_TYPE_IMAGE:
        widget_payload_image_create(&widget->payload.image);

        break;

    case WIDGET_TYPE_LIST:
        widget_payload_list_create(&widget->payload.list);

        break;

    case WIDGET_TYPE_SELECT:
        widget_payload_select_create(&widget->payload.select);

        break;

    case WIDGET_TYPE_SUBHEADER:
        widget_payload_subheader_create(&widget->payload.subheader);

        break;

    case WIDGET_TYPE_TABLE:
        widget_payload_table_create(&widget->payload.table);

        break;

    case WIDGET_TYPE_TEXT:
        widget_payload_text_create(&widget->payload.text);

        break;

    case WIDGET_TYPE_TOGGLE:
        widget_payload_toggle_create(&widget->payload.toggle);

        break;

    case WIDGET_TYPE_WINDOW:
        widget_payload_window_create(&widget->payload.window);

        break;

    }

}

void entity_destroy(struct widget *widget)
{

    switch (widget->header.type)
    {

    case WIDGET_TYPE_ANCHOR:
        widget_payload_anchor_destroy(&widget->payload.anchor);

        break;

    case WIDGET_TYPE_BUTTON:
        widget_payload_button_destroy(&widget->payload.button);

        break;

    case WIDGET_TYPE_CHOICE:
        widget_payload_choice_destroy(&widget->payload.choice);

        break;

    case WIDGET_TYPE_CODE:
        widget_payload_code_destroy(&widget->payload.code);

        break;

    case WIDGET_TYPE_DIVIDER:
        widget_payload_divider_destroy(&widget->payload.divider);

        break;

    case WIDGET_TYPE_FIELD:
        widget_payload_field_destroy(&widget->payload.field);

        break;

    case WIDGET_TYPE_HEADER:
        widget_payload_header_destroy(&widget->payload.header);

        break;

    case WIDGET_TYPE_IMAGE:
        widget_payload_image_destroy(&widget->payload.image);

        break;

    case WIDGET_TYPE_LIST:
        widget_payload_list_destroy(&widget->payload.list);

        break;

    case WIDGET_TYPE_SELECT:
        widget_payload_select_destroy(&widget->payload.select);

        break;

    case WIDGET_TYPE_SUBHEADER:
        widget_payload_subheader_destroy(&widget->payload.subheader);

        break;

    case WIDGET_TYPE_TABLE:
        widget_payload_table_destroy(&widget->payload.table);

        break;

    case WIDGET_TYPE_TEXT:
        widget_payload_text_destroy(&widget->payload.text);

        break;

    case WIDGET_TYPE_TOGGLE:
        widget_payload_toggle_destroy(&widget->payload.toggle);

        break;

    case WIDGET_TYPE_WINDOW:
        widget_payload_window_destroy(&widget->payload.window);

        break;

    }

    widget_header_destroy(&widget->header);

}

void entity_setstate(struct widget *widget, unsigned int state)
{

    switch (widget->header.type)
    {

    case WIDGET_TYPE_BUTTON:
        widget->header.state = widget_payload_button_setstate(&widget->header, state);

        break;

    case WIDGET_TYPE_FIELD:
        widget->header.state = widget_payload_field_setstate(&widget->header, state);

        break;

    case WIDGET_TYPE_SELECT:
        widget->header.state = widget_payload_select_setstate(&widget->header, state);

        break;

    case WIDGET_TYPE_TOGGLE:
        widget->header.state = widget_payload_toggle_setstate(&widget->header, state);

        break;

    default:
        widget->header.state = state;

        break;

    }

}

