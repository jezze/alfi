#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "attribute.h"
#include "widget.h"
#include "pool.h"

static void anchor_create(struct widget_payload_anchor *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_onclick_create(&payload->onclick, FUNCTION_NONE, "");
    attribute_target_create(&payload->target, ATTRIBUTE_TARGET_BLANK);

}

static void anchor_destroy(struct widget_payload_anchor *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_onclick_destroy(&payload->onclick);
    attribute_target_destroy(&payload->target);

}

static void button_create(struct widget_payload_button *payload)
{

    attribute_icon_create(&payload->icon, ATTRIBUTE_ICON_NONE);
    attribute_label_create(&payload->label, "");
    attribute_mode_create(&payload->mode, ATTRIBUTE_MODE_OFF);
    attribute_onclick_create(&payload->onclick, FUNCTION_NONE, "");
    attribute_target_create(&payload->target, ATTRIBUTE_TARGET_BLANK);

}

static void button_destroy(struct widget_payload_button *payload)
{

    attribute_icon_destroy(&payload->icon);
    attribute_label_destroy(&payload->label);
    attribute_mode_destroy(&payload->mode);
    attribute_onclick_destroy(&payload->onclick);
    attribute_target_destroy(&payload->target);

}

static unsigned int button_changestate(struct widget_header *header, unsigned int state)
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

static void choice_create(struct widget_payload_choice *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_mode_create(&payload->mode, ATTRIBUTE_MODE_OFF);

}

static void choice_destroy(struct widget_payload_choice *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_mode_destroy(&payload->mode);

}

static void code_create(struct widget_payload_code *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_link_create(&payload->link, "");

}

static void code_destroy(struct widget_payload_code *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_link_destroy(&payload->link);

}

static void divider_create(struct widget_payload_divider *payload)
{

    attribute_label_create(&payload->label, "");

}

static void divider_destroy(struct widget_payload_divider *payload)
{

    attribute_label_destroy(&payload->label);

}

static void field_create(struct widget_payload_field *payload)
{

    attribute_data_create(&payload->data, "");
    attribute_icon_create(&payload->icon, ATTRIBUTE_ICON_NONE);
    attribute_label_create(&payload->label, "");
    attribute_onlinebreak_create(&payload->onlinebreak, FUNCTION_NONE, "");
    attribute_range_create(&payload->range, 1, 1);
    attribute_type_create(&payload->type, ATTRIBUTE_TYPE_REGULAR);

}

static void field_destroy(struct widget_payload_field *payload)
{

    attribute_data_destroy(&payload->data);
    attribute_icon_destroy(&payload->icon);
    attribute_label_destroy(&payload->label);
    attribute_onlinebreak_destroy(&payload->onlinebreak);
    attribute_range_destroy(&payload->range);
    attribute_type_destroy(&payload->type);

}

static unsigned int field_changestate(struct widget_header *header, unsigned int state)
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

static void header_create(struct widget_payload_header *payload)
{

    attribute_label_create(&payload->label, "");

}

static void header_destroy(struct widget_payload_header *payload)
{

    attribute_label_destroy(&payload->label);

}

static void header2_create(struct widget_payload_header2 *payload)
{

    attribute_label_create(&payload->label, "");

}

static void header2_destroy(struct widget_payload_header2 *payload)
{

    attribute_label_destroy(&payload->label);

}

static void header3_create(struct widget_payload_header3 *payload)
{

    attribute_label_create(&payload->label, "");

}

static void header3_destroy(struct widget_payload_header3 *payload)
{

    attribute_label_destroy(&payload->label);

}

static void image_create(struct widget_payload_image *payload)
{

    attribute_link_create(&payload->link, "");

}

static void image_destroy(struct widget_payload_image *payload)
{

    attribute_link_destroy(&payload->link);

}

static void list_create(struct widget_payload_list *payload)
{

    attribute_label_create(&payload->label, "");

}

static void list_destroy(struct widget_payload_list *payload)
{

    attribute_label_destroy(&payload->label);

}

static void select_create(struct widget_payload_select *payload)
{

    attribute_data_create(&payload->data, "");
    attribute_label_create(&payload->label, "");
    attribute_range_create(&payload->range, 1, 1);

}

static void select_destroy(struct widget_payload_select *payload)
{

    attribute_data_destroy(&payload->data);
    attribute_label_destroy(&payload->label);
    attribute_range_destroy(&payload->range);

}

static unsigned int select_changestate(struct widget_header *header, unsigned int state)
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

static void table_create(struct widget_payload_table *payload)
{

    attribute_grid_create(&payload->grid, "");

}

static void table_destroy(struct widget_payload_table *payload)
{

    attribute_grid_destroy(&payload->grid);

}

static void text_create(struct widget_payload_text *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_link_create(&payload->link, "");

}

static void text_destroy(struct widget_payload_text *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_link_destroy(&payload->link);

}

static void toggle_create(struct widget_payload_toggle *payload)
{

    attribute_label_create(&payload->label, "");
    attribute_mode_create(&payload->mode, ATTRIBUTE_MODE_OFF);

}

static void toggle_destroy(struct widget_payload_toggle *payload)
{

    attribute_label_destroy(&payload->label);
    attribute_mode_destroy(&payload->mode);

}

static unsigned int toggle_changestate(struct widget_header *header, unsigned int state)
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

static void window_create(struct widget_payload_window *payload)
{

    attribute_label_create(&payload->label, "");

}

static void window_destroy(struct widget_payload_window *payload)
{

    attribute_label_destroy(&payload->label);

}

unsigned int widget_changestate(struct widget_header *header, unsigned int state)
{

    switch (header->type)
    {

    case WIDGET_TYPE_BUTTON:
        return header->state = button_changestate(header, state);

    case WIDGET_TYPE_FIELD:
        return header->state = field_changestate(header, state);

    case WIDGET_TYPE_SELECT:
        return header->state = select_changestate(header, state);

    case WIDGET_TYPE_TOGGLE:
        return header->state = toggle_changestate(header, state);

    }

    return header->state = state;

}

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

void widget_payload_create(union widget_payload *payload, unsigned int type)
{

    switch (type)
    {

    case WIDGET_TYPE_ANCHOR:
        anchor_create(&payload->anchor);

        break;

    case WIDGET_TYPE_BUTTON:
        button_create(&payload->button);

        break;

    case WIDGET_TYPE_CHOICE:
        choice_create(&payload->choice);

        break;

    case WIDGET_TYPE_CODE:
        code_create(&payload->code);

        break;

    case WIDGET_TYPE_DIVIDER:
        divider_create(&payload->divider);

        break;

    case WIDGET_TYPE_FIELD:
        field_create(&payload->field);

        break;

    case WIDGET_TYPE_HEADER:
        header_create(&payload->header);

        break;

    case WIDGET_TYPE_HEADER2:
        header2_create(&payload->header2);

        break;

    case WIDGET_TYPE_HEADER3:
        header3_create(&payload->header3);

        break;

    case WIDGET_TYPE_IMAGE:
        image_create(&payload->image);

        break;

    case WIDGET_TYPE_LIST:
        list_create(&payload->list);

        break;

    case WIDGET_TYPE_SELECT:
        select_create(&payload->select);

        break;

    case WIDGET_TYPE_TABLE:
        table_create(&payload->table);

        break;

    case WIDGET_TYPE_TEXT:
        text_create(&payload->text);

        break;

    case WIDGET_TYPE_TOGGLE:
        toggle_create(&payload->toggle);

        break;

    case WIDGET_TYPE_WINDOW:
        window_create(&payload->window);

        break;

    }

}

void widget_payload_destroy(union widget_payload *payload, unsigned int type)
{

    switch (type)
    {

    case WIDGET_TYPE_ANCHOR:
        anchor_destroy(&payload->anchor);

        break;

    case WIDGET_TYPE_BUTTON:
        button_destroy(&payload->button);

        break;

    case WIDGET_TYPE_CHOICE:
        choice_destroy(&payload->choice);

        break;

    case WIDGET_TYPE_CODE:
        code_destroy(&payload->code);

        break;

    case WIDGET_TYPE_DIVIDER:
        divider_destroy(&payload->divider);

        break;

    case WIDGET_TYPE_FIELD:
        field_destroy(&payload->field);

        break;

    case WIDGET_TYPE_HEADER:
        header_destroy(&payload->header);

        break;

    case WIDGET_TYPE_HEADER2:
        header2_destroy(&payload->header2);

        break;

    case WIDGET_TYPE_HEADER3:
        header3_destroy(&payload->header3);

        break;

    case WIDGET_TYPE_IMAGE:
        image_destroy(&payload->image);

        break;

    case WIDGET_TYPE_LIST:
        list_destroy(&payload->list);

        break;

    case WIDGET_TYPE_SELECT:
        select_destroy(&payload->select);

        break;

    case WIDGET_TYPE_TABLE:
        table_destroy(&payload->table);

        break;

    case WIDGET_TYPE_TEXT:
        text_destroy(&payload->text);

        break;

    case WIDGET_TYPE_TOGGLE:
        toggle_destroy(&payload->toggle);

        break;

    case WIDGET_TYPE_WINDOW:
        window_destroy(&payload->window);

        break;

    }

}

