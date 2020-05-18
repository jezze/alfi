#define WIDGET_TYPE_NONE                0
#define WIDGET_TYPE_ANCHOR              1
#define WIDGET_TYPE_AUDIO               2
#define WIDGET_TYPE_BUTTON              3
#define WIDGET_TYPE_CHOICE              4
#define WIDGET_TYPE_CODE                5
#define WIDGET_TYPE_DATETIME            6
#define WIDGET_TYPE_DIVIDER             7
#define WIDGET_TYPE_FIELD               8
#define WIDGET_TYPE_HEADER              9
#define WIDGET_TYPE_IMAGE               10
#define WIDGET_TYPE_LIST                11
#define WIDGET_TYPE_MAP                 12
#define WIDGET_TYPE_RICHTEXT            13
#define WIDGET_TYPE_SELECT              14
#define WIDGET_TYPE_SUBHEADER           15
#define WIDGET_TYPE_TABLE               16
#define WIDGET_TYPE_TEXT                17
#define WIDGET_TYPE_TOGGLE              18
#define WIDGET_TYPE_VIDEO               19
#define WIDGET_TYPE_WINDOW              20
#define WIDGET_STATE_NORMAL             0
#define WIDGET_STATE_HOVER              1
#define WIDGET_STATE_UNHOVER            2
#define WIDGET_STATE_FOCUS              3
#define WIDGET_STATE_UNFOCUS            4
#define WIDGET_FLAG_NONE                0
#define WIDGET_FLAG_FOCUSABLE           1

struct widget_header
{

    unsigned int type;
    unsigned int state;
    struct attribute_id id;
    struct attribute_in in;

};

struct widget_payload_anchor
{

    struct attribute_label label;
    struct attribute_link link;
    struct attribute_target target;

};

struct widget_payload_audio
{

    struct attribute_link link;

};

struct widget_payload_button
{

    struct attribute_icon icon;
    struct attribute_label label;
    struct attribute_link link;
    struct attribute_target target;
    struct attribute_mode mode;

};

struct widget_payload_choice
{

    struct attribute_label label;
    struct attribute_mode mode;

};

struct widget_payload_code
{

    struct attribute_label label;

};

struct widget_payload_divider
{

    struct attribute_label label;

};

struct widget_payload_field
{

    struct attribute_data data;
    struct attribute_icon icon;
    struct attribute_label label;
    struct attribute_range range;
    struct attribute_type type;

};

struct widget_payload_header
{

    struct attribute_label label;

};

struct widget_payload_image
{

    struct attribute_link link;

};

struct widget_payload_list
{

    struct attribute_label label;

};

struct widget_payload_select
{

    struct attribute_data data;
    struct attribute_label label;
    struct attribute_range range;

};

struct widget_payload_subheader
{

    struct attribute_label label;

};

struct widget_payload_table
{

    struct attribute_grid grid;

};

struct widget_payload_text
{

    struct attribute_label label;

};

struct widget_payload_toggle
{

    struct attribute_label label;
    struct attribute_mode mode;

};

struct widget_payload_window
{

    struct attribute_label label;

};

struct widget_payload_video
{

    struct attribute_link link;

};

union widget_payload
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

};

void widget_header_create(struct widget_header *header, unsigned int type, char *id, char *in);
void widget_header_destroy(struct widget_header *header);
void widget_payload_anchor_create(struct widget_payload_anchor *payload);
void widget_payload_anchor_destroy(struct widget_payload_anchor *payload);
void widget_payload_button_create(struct widget_payload_button *payload);
void widget_payload_button_destroy(struct widget_payload_button *payload);
unsigned int widget_payload_button_setstate(struct widget_header *header, unsigned int state);
void widget_payload_choice_create(struct widget_payload_choice *payload);
void widget_payload_choice_destroy(struct widget_payload_choice *payload);
void widget_payload_code_create(struct widget_payload_code *payload);
void widget_payload_code_destroy(struct widget_payload_code *payload);
void widget_payload_divider_create(struct widget_payload_divider *payload);
void widget_payload_divider_destroy(struct widget_payload_divider *payload);
void widget_payload_field_create(struct widget_payload_field *payload);
void widget_payload_field_destroy(struct widget_payload_field *payload);
unsigned int widget_payload_field_setstate(struct widget_header *header, unsigned int state);
void widget_payload_header_create(struct widget_payload_header *payload);
void widget_payload_header_destroy(struct widget_payload_header *payload);
void widget_payload_image_create(struct widget_payload_image *payload);
void widget_payload_image_destroy(struct widget_payload_image *payload);
void widget_payload_list_create(struct widget_payload_list *payload);
void widget_payload_list_destroy(struct widget_payload_list *payload);
void widget_payload_select_create(struct widget_payload_select *payload);
void widget_payload_select_destroy(struct widget_payload_select *payload);
unsigned int widget_payload_select_setstate(struct widget_header *header, unsigned int state);
void widget_payload_subheader_create(struct widget_payload_subheader *payload);
void widget_payload_subheader_destroy(struct widget_payload_subheader *payload);
void widget_payload_table_create(struct widget_payload_table *payload);
void widget_payload_table_destroy(struct widget_payload_table *payload);
void widget_payload_text_create(struct widget_payload_text *payload);
void widget_payload_text_destroy(struct widget_payload_text *payload);
void widget_payload_toggle_create(struct widget_payload_toggle *payload);
void widget_payload_toggle_destroy(struct widget_payload_toggle *payload);
unsigned int widget_payload_toggle_setstate(struct widget_header *header, unsigned int state);
void widget_payload_window_create(struct widget_payload_window *payload);
void widget_payload_window_destroy(struct widget_payload_window *payload);
