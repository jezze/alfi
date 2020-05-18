#define ATTRIBUTE_TYPE_NONE             0
#define ATTRIBUTE_TYPE_DATA             1
#define ATTRIBUTE_TYPE_GRID             2
#define ATTRIBUTE_TYPE_ICON             3
#define ATTRIBUTE_TYPE_ID               4
#define ATTRIBUTE_TYPE_IN               5
#define ATTRIBUTE_TYPE_LABEL            6
#define ATTRIBUTE_TYPE_LINK             7
#define ATTRIBUTE_TYPE_MODE             8
#define ATTRIBUTE_TYPE_RANGE            9
#define ATTRIBUTE_TYPE_TARGET           10
#define ATTRIBUTE_TYPE_TYPE             11
#define ATTRIBUTE_HALIGN_LEFT           1
#define ATTRIBUTE_HALIGN_CENTER         2
#define ATTRIBUTE_HALIGN_RIGHT          3
#define ATTRIBUTE_VALIGN_TOP            1
#define ATTRIBUTE_VALIGN_MIDDLE         2
#define ATTRIBUTE_VALIGN_BOTTOM         3
#define ATTRIBUTE_TARGET_BLANK          0
#define ATTRIBUTE_TARGET_SELF           1
#define ATTRIBUTE_MODE_OFF              0
#define ATTRIBUTE_MODE_ON               1
#define ATTRIBUTE_MODE_DISABLED         2
#define ATTRIBUTE_TYPE_REGULAR          0
#define ATTRIBUTE_TYPE_PASSWORD         1
#define ATTRIBUTE_ICON_BURGER           1
#define ATTRIBUTE_ICON_SEARCH           2

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

