union payload
{

    struct payload_anchor anchor;
    struct payload_audio audio;
    struct payload_button button;
    struct payload_choice choice;
    struct payload_code code;
    struct payload_divider divider;
    struct payload_field field;
    struct payload_header header;
    struct payload_image image;
    struct payload_list list;
    struct payload_select select;
    struct payload_subheader subheader;
    struct payload_table table;
    struct payload_text text;
    struct payload_toggle toggle;
    struct payload_video video;
    struct payload_window window;

};

struct widget
{

    struct list_item item;
    struct header header;
    union payload payload;
    struct frame frame;

};

unsigned int entity_checkflag(struct widget *widget, unsigned int flag);
void entity_createheader(struct widget *widget, unsigned int type, char *id, char *in);
void entity_destroyheader(struct widget *widget);
void entity_createpayload(struct widget *widget);
void entity_destroypayload(struct widget *widget);
void entity_setstate(struct widget *widget, unsigned int state);
