#define ALFI_COMMAND_NONE               0
#define ALFI_COMMAND_COMMENT            1
#define ALFI_COMMAND_DELETE             2
#define ALFI_COMMAND_INSERT             3
#define ALFI_COMMAND_UPDATE             4
#define ALFI_WIDGET_NONE                0
#define ALFI_WIDGET_AUDIO               1
#define ALFI_WIDGET_BUTTON              2
#define ALFI_WIDGET_CHOICE              3
#define ALFI_WIDGET_DATETIME            4
#define ALFI_WIDGET_DIVIDER             5
#define ALFI_WIDGET_FIELD               6
#define ALFI_WIDGET_HEADER              7
#define ALFI_WIDGET_HSTACK              8
#define ALFI_WIDGET_IMAGE               9
#define ALFI_WIDGET_LIST                10
#define ALFI_WIDGET_MAP                 11
#define ALFI_WIDGET_SELECT              12
#define ALFI_WIDGET_SUBHEADER           13
#define ALFI_WIDGET_TAB                 14
#define ALFI_WIDGET_TABLE               15
#define ALFI_WIDGET_TEXT                16
#define ALFI_WIDGET_VIDEO               17
#define ALFI_WIDGET_VSTACK              18
#define ALFI_WIDGET_WINDOW              19
#define ALFI_ATTRIBUTE_NONE             0
#define ALFI_ATTRIBUTE_GRID             1
#define ALFI_ATTRIBUTE_HALIGN           2
#define ALFI_ATTRIBUTE_ICON             3
#define ALFI_ATTRIBUTE_ID               4
#define ALFI_ATTRIBUTE_IN               5
#define ALFI_ATTRIBUTE_LABEL            6
#define ALFI_ATTRIBUTE_LINK             7
#define ALFI_ATTRIBUTE_RANGE            8
#define ALFI_ATTRIBUTE_TYPE             9
#define ALFI_ATTRIBUTE_VALIGN           10
#define ALFI_HALIGN_LEFT                1
#define ALFI_HALIGN_CENTER              2
#define ALFI_HALIGN_RIGHT               3
#define ALFI_VALIGN_TOP                 1
#define ALFI_VALIGN_MIDDLE              2
#define ALFI_VALIGN_BOTTOM              3
#define ALFI_TYPE_REGULAR               0
#define ALFI_TYPE_PASSWORD              1
#define ALFI_STATE_NORMAL               0
#define ALFI_STATE_HOVER                1
#define ALFI_STATE_ACTIVE               2
#define ALFI_ICON_BURGER                1
#define ALFI_ICON_SEARCH                2
#define ALFI_FLAG_CONTAINER             1
#define ALFI_FLAG_ITEM                  2

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

struct alfi_attribute_type
{

    int type;

};

struct alfi_attribute_valign
{

    unsigned int direction;

};

struct alfi_render_button
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;

    } background;

    struct
    {

        struct alfi_box box;
        struct alfi_color color;
        struct alfi_font font;

    } label;

};

struct alfi_render_divider
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;

    } border;

    struct
    {

        struct alfi_box box;
        struct alfi_color color;
        struct alfi_font font;

    } label;

};

struct alfi_render_field
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;

    } border;

    struct
    {

        struct alfi_box box;
        struct alfi_color color;
        struct alfi_font font;

    } label;

};

struct alfi_render_header
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;
        struct alfi_font font;

    } label;

};

struct alfi_render_image
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;

    } border;

};

struct alfi_render_list
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;

    } dot;

};

struct alfi_render_select
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;

    } background;

    struct
    {

        struct alfi_box box;
        struct alfi_color color;
        struct alfi_font font;

    } label;

};

struct alfi_render_subheader
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;
        struct alfi_font font;

    } label;

};

struct alfi_render_text
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;
        struct alfi_font font;

    } label;

};

struct alfi_render_window
{

    struct
    {

        struct alfi_box box;
        struct alfi_color color;

    } background;

};

struct alfi_widget_audio
{

    struct alfi_attribute_link link;

};

struct alfi_widget_button
{

    struct alfi_attribute_icon icon;
    struct alfi_attribute_label label;

};

struct alfi_widget_choice
{

    struct alfi_attribute_label label;

};

struct alfi_widget_divider
{

    struct alfi_attribute_label label;

};

struct alfi_widget_field
{

    struct alfi_attribute_icon icon;
    struct alfi_attribute_label label;
    struct alfi_attribute_range range;
    struct alfi_attribute_type type;

};

struct alfi_widget_header
{

    struct alfi_attribute_label label;

};

struct alfi_widget_hstack
{

    struct alfi_attribute_grid grid;
    struct alfi_attribute_halign halign;
    struct alfi_attribute_valign valign;

};

struct alfi_widget_image
{

    int ref;
    int w;
    int h;
    struct alfi_attribute_link link;

};

struct alfi_widget_list
{

    struct alfi_attribute_label label;

};

struct alfi_widget_select
{

    struct alfi_attribute_label label;
    struct alfi_attribute_range range;

};

struct alfi_widget_subheader
{

    struct alfi_attribute_label label;

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

};

struct alfi_widget_window
{

    struct alfi_attribute_label label;

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

struct alfi_widget
{

    unsigned int type;
    struct alfi_box bb;
    float cx;
    float cy;
    struct alfi_attribute_id id;
    struct alfi_attribute_in in;
    unsigned int state;
    struct list_item item;
    unsigned int group;

    union
    {

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

    } data;

    union
    {

        struct alfi_render_button button;
        struct alfi_render_divider divider;
        struct alfi_render_field field;
        struct alfi_render_header header;
        struct alfi_render_image image;
        struct alfi_render_list list;
        struct alfi_render_select select;
        struct alfi_render_subheader subheader;
        struct alfi_render_text text;
        struct alfi_render_window window;

    } animtarget, animcurrent;

};

