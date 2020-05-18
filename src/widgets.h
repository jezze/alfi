#define ALFI_WIDGET_NONE                0
#define ALFI_WIDGET_ANCHOR              1
#define ALFI_WIDGET_AUDIO               2
#define ALFI_WIDGET_BUTTON              3
#define ALFI_WIDGET_CHOICE              4
#define ALFI_WIDGET_CODE                5
#define ALFI_WIDGET_DATETIME            6
#define ALFI_WIDGET_DIVIDER             7
#define ALFI_WIDGET_FIELD               8
#define ALFI_WIDGET_HEADER              9
#define ALFI_WIDGET_IMAGE               10
#define ALFI_WIDGET_LIST                11
#define ALFI_WIDGET_MAP                 12
#define ALFI_WIDGET_RICHTEXT            13
#define ALFI_WIDGET_SELECT              14
#define ALFI_WIDGET_SUBHEADER           15
#define ALFI_WIDGET_TABLE               16
#define ALFI_WIDGET_TEXT                17
#define ALFI_WIDGET_TOGGLE              18
#define ALFI_WIDGET_VIDEO               19
#define ALFI_WIDGET_WINDOW              20

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
