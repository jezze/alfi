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

struct header
{

    unsigned int type;
    unsigned int state;
    struct attribute_id id;
    struct attribute_in in;

};

struct payload_anchor
{

    struct attribute_label label;
    struct attribute_link link;
    struct attribute_target target;

};

struct payload_audio
{

    struct attribute_link link;

};

struct payload_button
{

    struct attribute_icon icon;
    struct attribute_label label;
    struct attribute_link link;
    struct attribute_target target;
    struct attribute_mode mode;

};

struct payload_choice
{

    struct attribute_label label;
    struct attribute_mode mode;

};

struct payload_code
{

    struct attribute_label label;

};

struct payload_divider
{

    struct attribute_label label;

};

struct payload_field
{

    struct attribute_data data;
    struct attribute_icon icon;
    struct attribute_label label;
    struct attribute_range range;
    struct attribute_type type;

};

struct payload_header
{

    struct attribute_label label;

};

struct payload_image
{

    struct attribute_link link;

};

struct payload_list
{

    struct attribute_label label;

};

struct payload_select
{

    struct attribute_data data;
    struct attribute_label label;
    struct attribute_range range;

};

struct payload_subheader
{

    struct attribute_label label;

};

struct payload_table
{

    struct attribute_grid grid;

};

struct payload_text
{

    struct attribute_label label;

};

struct payload_toggle
{

    struct attribute_label label;
    struct attribute_mode mode;

};

struct payload_window
{

    struct attribute_label label;

};

struct payload_video
{

    struct attribute_link link;

};

