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
#define ALFI_WIDGET_TAB                 15
#define ALFI_WIDGET_TABLE               16
#define ALFI_WIDGET_TEXT                17
#define ALFI_WIDGET_VIDEO               18
#define ALFI_WIDGET_VSTACK              19
#define ALFI_WIDGET_WINDOW              20
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

struct alfi_frame_anchor
{

    struct alfi_style background;
    struct alfi_style label;

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

struct alfi_widget_anchor
{

    struct alfi_attribute_label label;
    struct alfi_attribute_link link;
    struct alfi_attribute_target target;
    struct alfi_frame_anchor frame;

};

struct alfi_widget_audio
{

    struct alfi_attribute_link link;

};

struct alfi_widget_button
{

    struct alfi_attribute_icon icon;
    struct alfi_attribute_label label;
    struct alfi_frame_button frame;

};

struct alfi_widget_choice
{

    struct alfi_attribute_label label;
    struct alfi_frame_choice frame;

};

struct alfi_widget_divider
{

    struct alfi_attribute_label label;
    struct alfi_frame_divider frame;

};

struct alfi_widget_field
{

    struct alfi_attribute_data data;
    struct alfi_attribute_icon icon;
    struct alfi_attribute_label label;
    struct alfi_attribute_range range;
    struct alfi_attribute_type type;
    struct alfi_frame_field frame;

};

struct alfi_widget_header
{

    struct alfi_attribute_label label;
    struct alfi_frame_header frame;

};

struct alfi_widget_hstack
{

    struct alfi_attribute_grid grid;
    struct alfi_attribute_halign halign;
    struct alfi_attribute_valign valign;

};

struct alfi_widget_image
{

    struct alfi_attribute_link link;
    struct alfi_resource resource;
    struct alfi_frame_image frame;

};

struct alfi_widget_list
{

    struct alfi_attribute_label label;
    struct alfi_frame_list frame;

};

struct alfi_widget_select
{

    struct alfi_attribute_data data;
    struct alfi_attribute_label label;
    struct alfi_attribute_range range;
    struct alfi_frame_select frame;

};

struct alfi_widget_subheader
{

    struct alfi_attribute_label label;
    struct alfi_frame_subheader frame;

};

struct alfi_widget_tab
{

    struct alfi_attribute_label label;

};

struct alfi_widget_table
{

    struct alfi_attribute_grid grid;

};

struct alfi_widget_text
{

    struct alfi_attribute_label label;
    struct alfi_frame_text frame;

};

struct alfi_widget_window
{

    struct alfi_attribute_label label;
    struct alfi_frame_window frame;

};

struct alfi_widget_video
{

    struct alfi_attribute_link link;

};

struct alfi_widget_vstack
{

    struct alfi_attribute_grid grid;
    struct alfi_attribute_halign halign;
    struct alfi_attribute_valign valign;

};

union alfi_payload
{

    struct alfi_widget_anchor anchor;
    struct alfi_widget_audio audio;
    struct alfi_widget_button button;
    struct alfi_widget_choice choice;
    struct alfi_widget_divider divider;
    struct alfi_widget_field field;
    struct alfi_widget_hstack hstack;
    struct alfi_widget_header header;
    struct alfi_widget_image image;
    struct alfi_widget_list list;
    struct alfi_widget_select select;
    struct alfi_widget_subheader subheader;
    struct alfi_widget_tab tab;
    struct alfi_widget_table table;
    struct alfi_widget_text text;
    struct alfi_widget_video video;
    struct alfi_widget_vstack vstack;
    struct alfi_widget_window window;

};

struct alfi_widget
{

    unsigned int type;
    struct alfi_box bb;
    struct alfi_attribute_id id;
    struct alfi_attribute_in in;
    unsigned int state;
    struct list_item item;
    union alfi_payload data;

};

