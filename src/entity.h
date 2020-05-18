struct widget
{

    struct list_item item;
    struct widget_header header;
    union payload
    {

        struct widget_payload_anchor anchor;
        struct widget_payload_audio audio;
        struct widget_payload_button button;
        struct widget_payload_choice choice;
        struct widget_payload_code code;
        struct widget_payload_divider divider;
        struct widget_payload_field field;
        struct widget_payload_header header;
        struct widget_payload_image image;
        struct widget_payload_list list;
        struct widget_payload_select select;
        struct widget_payload_subheader subheader;
        struct widget_payload_table table;
        struct widget_payload_text text;
        struct widget_payload_toggle toggle;
        struct widget_payload_video video;
        struct widget_payload_window window;

    } payload;

    struct frame frame;

};

unsigned int entity_checkflag(struct widget *widget, unsigned int flag);
void entity_createpayload(struct widget *widget);
void entity_destroypayload(struct widget *widget);
void entity_setstate(struct widget *widget, unsigned int state);
