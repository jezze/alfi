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
#define ALFI_WIDGET_IMAGE               9
#define ALFI_WIDGET_LIST                10
#define ALFI_WIDGET_MAP                 11
#define ALFI_WIDGET_SELECT              12
#define ALFI_WIDGET_STACK               13
#define ALFI_WIDGET_SUBHEADER           14
#define ALFI_WIDGET_TABLE               15
#define ALFI_WIDGET_TEXT                16
#define ALFI_WIDGET_TOGGLE              17
#define ALFI_WIDGET_VIDEO               18
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
#define ALFI_ATTRIBUTE_MODE             9
#define ALFI_ATTRIBUTE_RANGE            10
#define ALFI_ATTRIBUTE_TARGET           11
#define ALFI_ATTRIBUTE_TYPE             12
#define ALFI_ATTRIBUTE_VALIGN           13
#define ALFI_HALIGN_LEFT                1
#define ALFI_HALIGN_CENTER              2
#define ALFI_HALIGN_RIGHT               3
#define ALFI_VALIGN_TOP                 1
#define ALFI_VALIGN_MIDDLE              2
#define ALFI_VALIGN_BOTTOM              3
#define ALFI_TARGET_BLANK               0
#define ALFI_TARGET_SELF                1
#define ALFI_MODE_OFF                   0
#define ALFI_MODE_ON                    1
#define ALFI_MODE_DISABLED              2
#define ALFI_TYPE_REGULAR               0
#define ALFI_TYPE_PASSWORD              1
#define ALFI_STATE_NORMAL               0
#define ALFI_STATE_HOVER                1
#define ALFI_STATE_UNHOVER              2
#define ALFI_STATE_FOCUS                3
#define ALFI_STATE_UNFOCUS              4
#define ALFI_ICON_BURGER                1
#define ALFI_ICON_SEARCH                2
#define ALFI_FLAG_NONE                  0
#define ALFI_FLAG_FOCUSABLE             1
#define ALFI_DATASIZE                   128

struct attribute_data
{

    char *content;
    unsigned int offset;
    unsigned int total;

};

struct attribute_grid
{

    char *format;

};

struct attribute_halign
{

    unsigned int direction;

};

struct attribute_icon
{

    unsigned int type;

};

struct attribute_id
{

    char *name;

};

struct attribute_in
{

    char *name;

};

struct attribute_label
{

    char *content;

};

struct attribute_link
{

    char *url;
    char *mime;

};

struct attribute_mode
{

    int mode;

};

struct attribute_range
{

    int min;
    int max;

};

struct attribute_target
{

    int type;

};

struct attribute_type
{

    int type;

};

struct attribute_valign
{

    unsigned int direction;

};

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

struct payload_stack
{

    struct attribute_halign halign;
    struct attribute_valign valign;

};

union payload
{

    struct payload_anchor anchor;
    struct payload_audio audio;
    struct payload_button button;
    struct payload_choice choice;
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
    struct payload_stack stack;
    struct payload_window window;

};

struct frame
{

    struct style_box bounds;
    struct style styles[8];
    unsigned int nstyles;
    unsigned int animating;

};

struct widget
{

    struct list_item item;
    struct header header;
    union payload payload;
    struct frame frame;

};

unsigned int widgets_checkflag(struct widget *widget, unsigned int flag);
void widgets_createheader(struct widget *widget, unsigned int type, char *id, char *in);
void widgets_destroyheader(struct widget *widget);
void widgets_createpayload(struct widget *widget);
void widgets_destroypayload(struct widget *widget);
void widgets_setstate(struct widget *widget, unsigned int state);
void widgets_setup(void);
