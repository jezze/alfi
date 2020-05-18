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
        widget_payload_anchor_create(&widget->payload.anchor);

        break;

    case ALFI_WIDGET_BUTTON:
        widget_payload_button_create(&widget->payload.button);

        break;

    case ALFI_WIDGET_CHOICE:
        widget_payload_choice_create(&widget->payload.choice);

        break;

    case ALFI_WIDGET_CODE:
        widget_payload_code_create(&widget->payload.code);

        break;

    case ALFI_WIDGET_DIVIDER:
        widget_payload_divider_create(&widget->payload.divider);

        break;

    case ALFI_WIDGET_FIELD:
        widget_payload_field_create(&widget->payload.field);

        break;

    case ALFI_WIDGET_HEADER:
        widget_payload_header_create(&widget->payload.header);

        break;

    case ALFI_WIDGET_IMAGE:
        widget_payload_image_create(&widget->payload.image);

        break;

    case ALFI_WIDGET_LIST:
        widget_payload_list_create(&widget->payload.list);

        break;

    case ALFI_WIDGET_SELECT:
        widget_payload_select_create(&widget->payload.select);

        break;

    case ALFI_WIDGET_SUBHEADER:
        widget_payload_subheader_create(&widget->payload.subheader);

        break;

    case ALFI_WIDGET_TABLE:
        widget_payload_table_create(&widget->payload.table);

        break;

    case ALFI_WIDGET_TEXT:
        widget_payload_text_create(&widget->payload.text);

        break;

    case ALFI_WIDGET_TOGGLE:
        widget_payload_toggle_create(&widget->payload.toggle);

        break;

    case ALFI_WIDGET_WINDOW:
        widget_payload_window_create(&widget->payload.window);

        break;

    }

}

void entity_destroypayload(struct widget *widget)
{

    switch (widget->header.type)
    {

    case ALFI_WIDGET_ANCHOR:
        widget_payload_anchor_destroy(&widget->payload.anchor);

        break;

    case ALFI_WIDGET_BUTTON:
        widget_payload_button_destroy(&widget->payload.button);

        break;

    case ALFI_WIDGET_CHOICE:
        widget_payload_choice_destroy(&widget->payload.choice);

        break;

    case ALFI_WIDGET_CODE:
        widget_payload_code_destroy(&widget->payload.code);

        break;

    case ALFI_WIDGET_DIVIDER:
        widget_payload_divider_destroy(&widget->payload.divider);

        break;

    case ALFI_WIDGET_FIELD:
        widget_payload_field_destroy(&widget->payload.field);

        break;

    case ALFI_WIDGET_HEADER:
        widget_payload_header_destroy(&widget->payload.header);

        break;

    case ALFI_WIDGET_IMAGE:
        widget_payload_image_destroy(&widget->payload.image);

        break;

    case ALFI_WIDGET_LIST:
        widget_payload_list_destroy(&widget->payload.list);

        break;

    case ALFI_WIDGET_SELECT:
        widget_payload_select_destroy(&widget->payload.select);

        break;

    case ALFI_WIDGET_SUBHEADER:
        widget_payload_subheader_destroy(&widget->payload.subheader);

        break;

    case ALFI_WIDGET_TABLE:
        widget_payload_table_destroy(&widget->payload.table);

        break;

    case ALFI_WIDGET_TEXT:
        widget_payload_text_destroy(&widget->payload.text);

        break;

    case ALFI_WIDGET_TOGGLE:
        widget_payload_toggle_destroy(&widget->payload.toggle);

        break;

    case ALFI_WIDGET_WINDOW:
        widget_payload_window_destroy(&widget->payload.window);

        break;

    }

}

void entity_setstate(struct widget *widget, unsigned int state)
{

    switch (widget->header.type)
    {

    case ALFI_WIDGET_BUTTON:
        widget->header.state = widget_payload_button_setstate(&widget->header, state);

        break;

    case ALFI_WIDGET_FIELD:
        widget->header.state = widget_payload_field_setstate(&widget->header, state);

        break;

    case ALFI_WIDGET_SELECT:
        widget->header.state = widget_payload_select_setstate(&widget->header, state);

        break;

    case ALFI_WIDGET_TOGGLE:
        widget->header.state = widget_payload_toggle_setstate(&widget->header, state);

        break;

    default:
        widget->header.state = ALFI_STATE_NORMAL;

        break;

    }

}

