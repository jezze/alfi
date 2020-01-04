#define ALFI_COMMAND_NONE               0
#define ALFI_COMMAND_COMMENT            1
#define ALFI_COMMAND_DELETE             2
#define ALFI_COMMAND_INSERT             3
#define ALFI_COMMAND_UPDATE             4
#define ALFI_WIDGET_NONE                0
#define ALFI_WIDGET_ANCHOR              1
#define ALFI_WIDGET_AUDIO               2
#define ALFI_WIDGET_BUTTON              3
#define ALFI_WIDGET_CHOICE              4
#define ALFI_WIDGET_DATETIME            5
#define ALFI_WIDGET_DIVIDER             6
#define ALFI_WIDGET_FIELD               7
#define ALFI_WIDGET_HEADER              8
#define ALFI_WIDGET_HSTACK              9
#define ALFI_WIDGET_IMAGE               10
#define ALFI_WIDGET_LIST                11
#define ALFI_WIDGET_MAP                 12
#define ALFI_WIDGET_SELECT              13
#define ALFI_WIDGET_SUBHEADER           14
#define ALFI_WIDGET_TABLE               15
#define ALFI_WIDGET_TEXT                16
#define ALFI_WIDGET_VIDEO               17
#define ALFI_WIDGET_VSTACK              18
#define ALFI_WIDGET_WINDOW              19
#define ALFI_ATTRIBUTE_NONE             0
#define ALFI_ATTRIBUTE_DATA             1
#define ALFI_ATTRIBUTE_GRID             2
#define ALFI_ATTRIBUTE_HALIGN           3
#define ALFI_ATTRIBUTE_ICON             4
#define ALFI_ATTRIBUTE_ID               5
#define ALFI_ATTRIBUTE_IN               6
#define ALFI_ATTRIBUTE_LABEL            7
#define ALFI_ATTRIBUTE_LINK             8
#define ALFI_ATTRIBUTE_RANGE            9
#define ALFI_ATTRIBUTE_TARGET           10
#define ALFI_ATTRIBUTE_TYPE             11
#define ALFI_ATTRIBUTE_VALIGN           12
#define ALFI_HALIGN_LEFT                1
#define ALFI_HALIGN_CENTER              2
#define ALFI_HALIGN_RIGHT               3
#define ALFI_VALIGN_TOP                 1
#define ALFI_VALIGN_MIDDLE              2
#define ALFI_VALIGN_BOTTOM              3
#define ALFI_TARGET_BLANK               0
#define ALFI_TARGET_SELF                1
#define ALFI_TYPE_REGULAR               0
#define ALFI_TYPE_PASSWORD              1
#define ALFI_STATE_NORMAL               0
#define ALFI_STATE_HOVER                1
#define ALFI_STATE_UNHOVER              2
#define ALFI_STATE_FOCUS                3
#define ALFI_STATE_UNFOCUS              4
#define ALFI_CURSOR_ARROW               0
#define ALFI_CURSOR_IBEAM               1
#define ALFI_CURSOR_HAND                2
#define ALFI_ICON_BURGER                1
#define ALFI_ICON_SEARCH                2
#define ALFI_FLAG_CONTAINER             1
#define ALFI_FLAG_ITEM                  2
#define ALFI_FLAG_FOCUSABLE             4
#define ALFI_DATASIZE                   128

struct alfi_attribute_data
{

    char *content;
    unsigned int offset;
    unsigned int total;

};

struct alfi_attribute_grid
{

    unsigned int csize;
    unsigned int rsize;
    unsigned int coffset;
    unsigned int roffset;
    unsigned int clength;
    unsigned int rlength;

};

struct alfi_attribute_halign
{

    unsigned int direction;

};

struct alfi_attribute_icon
{

    unsigned int type;

};

struct alfi_attribute_id
{

    char *name;

};

struct alfi_attribute_in
{

    char *name;

};

struct alfi_attribute_label
{

    char *content;

};

struct alfi_attribute_link
{

    char *url;
    char *mime;

};

struct alfi_attribute_range
{

    int min;
    int max;

};

struct alfi_attribute_target
{

    int type;

};

struct alfi_attribute_type
{

    int type;

};

struct alfi_attribute_valign
{

    unsigned int direction;

};

struct alfi_resource
{

    int ref;
    int w;
    int h;

};

struct alfi_frame_anchor
{

    struct alfi_style background;
    struct alfi_style label;

};

struct alfi_frame_button
{

    struct alfi_style background;
    struct alfi_style border;
    struct alfi_style label;

};

struct alfi_frame_choice
{

    struct alfi_style background;
    struct alfi_style label;

};

struct alfi_frame_divider
{

    struct alfi_style background;
    struct alfi_style border;
    struct alfi_style label;

};

struct alfi_frame_field
{

    struct alfi_style background;
    struct alfi_style border;
    struct alfi_style label;
    struct alfi_style data;

};

struct alfi_frame_header
{

    struct alfi_style background;
    struct alfi_style label;

};

struct alfi_frame_image
{

    struct alfi_style background;
    struct alfi_style frame;

};

struct alfi_frame_list
{

    struct alfi_style dot;

};

struct alfi_frame_select
{

    struct alfi_style background;
    struct alfi_style border;
    struct alfi_style label;
    struct alfi_style data;

};

struct alfi_frame_subheader
{

    struct alfi_style background;
    struct alfi_style label;

};

struct alfi_frame_text
{

    struct alfi_style background;
    struct alfi_style label;

};

struct alfi_frame_window
{

    struct alfi_style background;

};

union alfi_frame
{

    struct alfi_frame_button button;
    struct alfi_frame_choice choice;
    struct alfi_frame_divider divider;
    struct alfi_frame_field field;
    struct alfi_frame_header header;
    struct alfi_frame_image image;
    struct alfi_frame_anchor anchor;
    struct alfi_frame_list list;
    struct alfi_frame_select select;
    struct alfi_frame_subheader subheader;
    struct alfi_frame_text text;
    struct alfi_frame_window window;

};

struct alfi_header
{

    unsigned int type;
    struct alfi_attribute_id id;
    struct alfi_attribute_in in;

};

struct alfi_payload_anchor
{

    struct alfi_attribute_label label;
    struct alfi_attribute_link link;
    struct alfi_attribute_target target;

};

struct alfi_payload_audio
{

    struct alfi_attribute_link link;

};

struct alfi_payload_button
{

    struct alfi_attribute_icon icon;
    struct alfi_attribute_label label;
    struct alfi_attribute_link link;
    struct alfi_attribute_target target;

};

struct alfi_payload_choice
{

    struct alfi_attribute_label label;

};

struct alfi_payload_divider
{

    struct alfi_attribute_label label;

};

struct alfi_payload_field
{

    struct alfi_attribute_data data;
    struct alfi_attribute_icon icon;
    struct alfi_attribute_label label;
    struct alfi_attribute_range range;
    struct alfi_attribute_type type;

};

struct alfi_payload_header
{

    struct alfi_attribute_label label;

};

struct alfi_payload_hstack
{

    struct alfi_attribute_grid grid;
    struct alfi_attribute_halign halign;
    struct alfi_attribute_valign valign;

};

struct alfi_payload_image
{

    struct alfi_attribute_link link;
    struct alfi_resource resource;

};

struct alfi_payload_list
{

    struct alfi_attribute_label label;

};

struct alfi_payload_select
{

    struct alfi_attribute_data data;
    struct alfi_attribute_label label;
    struct alfi_attribute_range range;

};

struct alfi_payload_subheader
{

    struct alfi_attribute_label label;

};

struct alfi_payload_table
{

    struct alfi_attribute_grid grid;

};

struct alfi_payload_text
{

    struct alfi_attribute_label label;

};

struct alfi_payload_window
{

    struct alfi_attribute_label label;

};

struct alfi_payload_video
{

    struct alfi_attribute_link link;

};

struct alfi_payload_vstack
{

    struct alfi_attribute_grid grid;
    struct alfi_attribute_halign halign;
    struct alfi_attribute_valign valign;

};

union alfi_payload
{

    struct alfi_payload_anchor anchor;
    struct alfi_payload_audio audio;
    struct alfi_payload_button button;
    struct alfi_payload_choice choice;
    struct alfi_payload_divider divider;
    struct alfi_payload_field field;
    struct alfi_payload_hstack hstack;
    struct alfi_payload_header header;
    struct alfi_payload_image image;
    struct alfi_payload_list list;
    struct alfi_payload_select select;
    struct alfi_payload_subheader subheader;
    struct alfi_payload_table table;
    struct alfi_payload_text text;
    struct alfi_payload_video video;
    struct alfi_payload_vstack vstack;
    struct alfi_payload_window window;

};

struct alfi_widget
{

    struct alfi_box bb;
    unsigned int state;
    struct list_item item;
    struct alfi_header header;
    union alfi_payload payload;
    union alfi_frame frame;

};

