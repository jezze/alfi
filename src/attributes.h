#define ALFI_ATTRIBUTE_NONE             0
#define ALFI_ATTRIBUTE_DATA             1
#define ALFI_ATTRIBUTE_GRID             2
#define ALFI_ATTRIBUTE_ICON             3
#define ALFI_ATTRIBUTE_ID               4
#define ALFI_ATTRIBUTE_IN               5
#define ALFI_ATTRIBUTE_LABEL            6
#define ALFI_ATTRIBUTE_LINK             7
#define ALFI_ATTRIBUTE_MODE             8
#define ALFI_ATTRIBUTE_RANGE            9
#define ALFI_ATTRIBUTE_TARGET           10
#define ALFI_ATTRIBUTE_TYPE             11
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

    int type;

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

