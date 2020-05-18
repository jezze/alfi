#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "attributes.h"
#include "widgets.h"
#include "pool.h"

void widget_header_create(struct widget_header *header, unsigned int type, char *id, char *in)
{

    memset(header, 0, sizeof (struct widget_header));

    header->type = type;
    header->id.name = pool_string_create(ALFI_ATTRIBUTE_ID, header->id.name, id);
    header->in.name = pool_string_create(ALFI_ATTRIBUTE_IN, header->in.name, in);

}

void widget_header_destroy(struct widget_header *header)
{

    header->id.name = pool_string_destroy(ALFI_ATTRIBUTE_ID, header->id.name);
    header->in.name = pool_string_destroy(ALFI_ATTRIBUTE_IN, header->in.name);

}

void widget_payload_anchor_create(struct widget_payload_anchor *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

void widget_payload_anchor_destroy(struct widget_payload_anchor *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

void widget_payload_button_create(struct widget_payload_button *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

void widget_payload_button_destroy(struct widget_payload_button *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

unsigned int widget_payload_button_setstate(struct widget_header *header, unsigned int state)
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

void widget_payload_choice_create(struct widget_payload_choice *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

void widget_payload_choice_destroy(struct widget_payload_choice *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

void widget_payload_code_create(struct widget_payload_code *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

void widget_payload_code_destroy(struct widget_payload_code *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

void widget_payload_divider_create(struct widget_payload_divider *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

void widget_payload_divider_destroy(struct widget_payload_divider *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

void widget_payload_field_create(struct widget_payload_field *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

void widget_payload_field_destroy(struct widget_payload_field *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

unsigned int widget_payload_field_setstate(struct widget_header *header, unsigned int state)
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

void widget_payload_header_create(struct widget_payload_header *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

void widget_payload_header_destroy(struct widget_payload_header *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

void widget_payload_image_create(struct widget_payload_image *payload)
{

}

void widget_payload_image_destroy(struct widget_payload_image *payload)
{

}

void widget_payload_list_create(struct widget_payload_list *payload)
{

}

void widget_payload_list_destroy(struct widget_payload_list *payload)
{

}

void widget_payload_select_create(struct widget_payload_select *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

void widget_payload_select_destroy(struct widget_payload_select *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

unsigned int widget_payload_select_setstate(struct widget_header *header, unsigned int state)
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

void widget_payload_subheader_create(struct widget_payload_subheader *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

void widget_payload_subheader_destroy(struct widget_payload_subheader *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

void widget_payload_table_create(struct widget_payload_table *payload)
{

    payload->grid.format = pool_string_create(ALFI_ATTRIBUTE_GRID, payload->grid.format, "12LT");

}

void widget_payload_table_destroy(struct widget_payload_table *payload)
{

    payload->grid.format = pool_string_destroy(ALFI_ATTRIBUTE_GRID, payload->grid.format);

}

void widget_payload_text_create(struct widget_payload_text *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

void widget_payload_text_destroy(struct widget_payload_text *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

void widget_payload_toggle_create(struct widget_payload_toggle *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

void widget_payload_toggle_destroy(struct widget_payload_toggle *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

unsigned int widget_payload_toggle_setstate(struct widget_header *header, unsigned int state)
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

void widget_payload_window_create(struct widget_payload_window *payload)
{

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");

}

void widget_payload_window_destroy(struct widget_payload_window *payload)
{

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

