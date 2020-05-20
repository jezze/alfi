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

    header->type = type;
    header->state = WIDGET_STATE_NORMAL;

    attribute_id_create(&header->id, id);
    attribute_in_create(&header->in, in);

}

void widget_header_destroy(struct widget_header *header)
{

    attribute_id_destroy(&header->id);
    attribute_in_destroy(&header->in);

}

void widget_payload_anchor_create(struct widget_payload_anchor *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_link_create(&payload->link, "", "");
    attribute_target_create(&payload->target, ATTRIBUTE_TARGET_BLANK);

}

void widget_payload_anchor_destroy(struct widget_payload_anchor *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_link_destroy(&payload->link);
    attribute_target_destroy(&payload->target);

}

void widget_payload_button_create(struct widget_payload_button *payload)
{

    attribute_icon_create(&payload->icon, ATTRIBUTE_ICON_NONE);
    attribute_label_create(&payload->label, "undefined");
    attribute_link_create(&payload->link, "", "");
    attribute_target_create(&payload->target, ATTRIBUTE_TARGET_BLANK);
    attribute_mode_create(&payload->mode, ATTRIBUTE_MODE_OFF);

}

void widget_payload_button_destroy(struct widget_payload_button *payload)
{

    attribute_icon_destroy(&payload->icon);
    attribute_label_destroy(&payload->label);
    attribute_link_destroy(&payload->link);
    attribute_target_destroy(&payload->target);
    attribute_mode_destroy(&payload->mode);

}

unsigned int widget_payload_button_setstate(struct widget_header *header, unsigned int state)
{

    switch (state)
    {

    case WIDGET_STATE_HOVER:
    case WIDGET_STATE_UNHOVER:
        if (header->state == WIDGET_STATE_FOCUS)
            return WIDGET_STATE_FOCUS;

    }

    return state;

}

void widget_payload_choice_create(struct widget_payload_choice *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_mode_create(&payload->mode, ATTRIBUTE_MODE_OFF);

}

void widget_payload_choice_destroy(struct widget_payload_choice *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_mode_destroy(&payload->mode);

}

void widget_payload_code_create(struct widget_payload_code *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_link_create(&payload->link, "", "");

}

void widget_payload_code_destroy(struct widget_payload_code *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_link_destroy(&payload->link);

}

void widget_payload_divider_create(struct widget_payload_divider *payload)
{

    attribute_label_create(&payload->label, "");

}

void widget_payload_divider_destroy(struct widget_payload_divider *payload)
{

    attribute_label_destroy(&payload->label);

}

void widget_payload_field_create(struct widget_payload_field *payload)
{

    attribute_data_create(&payload->data, "");
    attribute_icon_create(&payload->icon, ATTRIBUTE_ICON_NONE);
    attribute_label_create(&payload->label, "");
    attribute_range_create(&payload->range, 1, 1);
    attribute_type_create(&payload->type, ATTRIBUTE_TYPE_REGULAR);

}

void widget_payload_field_destroy(struct widget_payload_field *payload)
{

    attribute_data_destroy(&payload->data);
    attribute_icon_destroy(&payload->icon);
    attribute_label_destroy(&payload->label);
    attribute_range_destroy(&payload->range);
    attribute_type_destroy(&payload->type);

}

unsigned int widget_payload_field_setstate(struct widget_header *header, unsigned int state)
{

    switch (state)
    {

    case WIDGET_STATE_HOVER:
    case WIDGET_STATE_UNHOVER:
        if (header->state == WIDGET_STATE_FOCUS)
            return WIDGET_STATE_FOCUS;

    }

    return state;

}

void widget_payload_header_create(struct widget_payload_header *payload)
{

    attribute_label_create(&payload->label, "");

}

void widget_payload_header_destroy(struct widget_payload_header *payload)
{

    attribute_label_destroy(&payload->label);

}

void widget_payload_image_create(struct widget_payload_image *payload)
{

    attribute_link_create(&payload->link, "", "");

}

void widget_payload_image_destroy(struct widget_payload_image *payload)
{

    attribute_link_destroy(&payload->link);

}

void widget_payload_list_create(struct widget_payload_list *payload)
{

    attribute_label_create(&payload->label, "");

}

void widget_payload_list_destroy(struct widget_payload_list *payload)
{

    attribute_label_destroy(&payload->label);

}

void widget_payload_select_create(struct widget_payload_select *payload)
{

    attribute_data_create(&payload->data, "");
    attribute_label_create(&payload->label, "");
    attribute_range_create(&payload->range, 1, 1);

}

void widget_payload_select_destroy(struct widget_payload_select *payload)
{

    attribute_data_destroy(&payload->data);
    attribute_label_destroy(&payload->label);
    attribute_range_destroy(&payload->range);

}

unsigned int widget_payload_select_setstate(struct widget_header *header, unsigned int state)
{

    switch (state)
    {

    case WIDGET_STATE_HOVER:
    case WIDGET_STATE_UNHOVER:
        if (header->state == WIDGET_STATE_FOCUS)
            return WIDGET_STATE_FOCUS;

    }

    return state;

}

void widget_payload_subheader_create(struct widget_payload_subheader *payload)
{

    attribute_label_create(&payload->label, "");

}

void widget_payload_subheader_destroy(struct widget_payload_subheader *payload)
{

    attribute_label_destroy(&payload->label);

}

void widget_payload_table_create(struct widget_payload_table *payload)
{

    attribute_grid_create(&payload->grid, "12LT");

}

void widget_payload_table_destroy(struct widget_payload_table *payload)
{

    attribute_grid_destroy(&payload->grid);

}

void widget_payload_text_create(struct widget_payload_text *payload)
{

    attribute_label_create(&payload->label, "");

}

void widget_payload_text_destroy(struct widget_payload_text *payload)
{

    attribute_label_destroy(&payload->label);

}

void widget_payload_toggle_create(struct widget_payload_toggle *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_mode_create(&payload->mode, ATTRIBUTE_MODE_OFF);

}

void widget_payload_toggle_destroy(struct widget_payload_toggle *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_mode_destroy(&payload->mode);

}

unsigned int widget_payload_toggle_setstate(struct widget_header *header, unsigned int state)
{

    switch (state)
    {

    case WIDGET_STATE_HOVER:
    case WIDGET_STATE_UNHOVER:
        if (header->state == WIDGET_STATE_FOCUS)
            return WIDGET_STATE_FOCUS;

    }

    return state;

}

void widget_payload_window_create(struct widget_payload_window *payload)
{

    attribute_label_create(&payload->label, "");

}

void widget_payload_window_destroy(struct widget_payload_window *payload)
{

    attribute_label_destroy(&payload->label);

}

