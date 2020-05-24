#define ATTRIBUTE_DATA                  1
#define ATTRIBUTE_GRID                  2
#define ATTRIBUTE_ICON                  3
#define ATTRIBUTE_ICON_NONE             0
#define ATTRIBUTE_ICON_ALARM            0xEEA3
#define ATTRIBUTE_ICON_AT               0xEEA6
#define ATTRIBUTE_ICON_ATTACHMENT       0xEEA7
#define ATTRIBUTE_ICON_AUDIO            0xEEA8
#define ATTRIBUTE_ICON_BAN              0xEEAD
#define ATTRIBUTE_ICON_BIN              0xEEBB
#define ATTRIBUTE_ICON_BOOKMARK         0xEEC0
#define ATTRIBUTE_ICON_BUG              0xEEC7
#define ATTRIBUTE_ICON_CART             0xEED2
#define ATTRIBUTE_ICON_CHAT             0xEED5
#define ATTRIBUTE_ICON_CHECK            0xEED8
#define ATTRIBUTE_ICON_CLIP             0xEEDB
#define ATTRIBUTE_ICON_CLOCK            0xEEDC
#define ATTRIBUTE_ICON_CLOSE            0xEEE4
#define ATTRIBUTE_ICON_COMMENT          0xEEEB
#define ATTRIBUTE_ICON_COMPASS          0xEEED
#define ATTRIBUTE_ICON_COMPUTER         0xEEEE
#define ATTRIBUTE_ICON_CONSOLE          0xEEF0
#define ATTRIBUTE_ICON_DOWNLOAD         0xEF08
#define ATTRIBUTE_ICON_EARTH            0xEF0E
#define ATTRIBUTE_ICON_EDIT             0xEF10
#define ATTRIBUTE_ICON_ENVELOPE         0xEF14
#define ATTRIBUTE_ICON_ERROR            0xEF16
#define ATTRIBUTE_ICON_EXCLAMATION      0xEF19
#define ATTRIBUTE_ICON_EYE              0xEF21
#define ATTRIBUTE_ICON_FAVOURITE        0xEF25
#define ATTRIBUTE_ICON_FLAG             0xEF2F
#define ATTRIBUTE_ICON_HEART            0xEF45
#define ATTRIBUTE_ICON_HISTORY          0xEF46
#define ATTRIBUTE_ICON_IMAGE            0xEF4B
#define ATTRIBUTE_ICON_INFO             0xEF4E
#define ATTRIBUTE_ICON_KEY              0xEF59
#define ATTRIBUTE_ICON_LIKE             0xEF6E
#define ATTRIBUTE_ICON_LINK             0xEF71
#define ATTRIBUTE_ICON_LOCK             0xEF7A
#define ATTRIBUTE_ICON_LOGIN            0xEF7B
#define ATTRIBUTE_ICON_LOGOUT           0xEF7C
#define ATTRIBUTE_ICON_MAGNET           0xEF86
#define ATTRIBUTE_ICON_MARKER           0xEF79
#define ATTRIBUTE_ICON_MENU             0xEFA2
#define ATTRIBUTE_ICON_MINUS            0xEF9A
#define ATTRIBUTE_ICON_OPTIONS          0xEFB0
#define ATTRIBUTE_ICON_PLUS             0xEFC2
#define ATTRIBUTE_ICON_POWER            0xEFC4
#define ATTRIBUTE_ICON_PRINT            0xEFC6
#define ATTRIBUTE_ICON_QUESTION         0xEFCA
#define ATTRIBUTE_ICON_REFRESH          0xEFD1
#define ATTRIBUTE_ICON_REPLY            0xEFD4
#define ATTRIBUTE_ICON_REPLYALL         0xEFD3
#define ATTRIBUTE_ICON_RETWEET          0xEFD7
#define ATTRIBUTE_ICON_SEARCH           0xED1B
#define ATTRIBUTE_ICON_SHARE            0xEFE5
#define ATTRIBUTE_ICON_SORT             0xEFEF
#define ATTRIBUTE_ICON_TAG              0xF004
#define ATTRIBUTE_ICON_TAGS             0xF005
#define ATTRIBUTE_ICON_THUMBDOWN        0xF00B
#define ATTRIBUTE_ICON_THUMBUP          0xF00C
#define ATTRIBUTE_ICON_UNLOCK           0xF01B
#define ATTRIBUTE_ICON_UPLOAD           0xF01C
#define ATTRIBUTE_ICON_WARNING          0xF025
#define ATTRIBUTE_ID                    4
#define ATTRIBUTE_IN                    5
#define ATTRIBUTE_LABEL                 6
#define ATTRIBUTE_LINK                  7
#define ATTRIBUTE_MODE                  8
#define ATTRIBUTE_MODE_OFF              0
#define ATTRIBUTE_MODE_ON               1
#define ATTRIBUTE_MODE_DISABLED         2
#define ATTRIBUTE_ONCLICK               9
#define ATTRIBUTE_ONCLICK_NONE          0
#define ATTRIBUTE_ONCLICK_ALFI          1
#define ATTRIBUTE_ONCLICK_GET           2
#define ATTRIBUTE_ONCLICK_POST          3
#define ATTRIBUTE_RANGE                 10
#define ATTRIBUTE_TARGET                11
#define ATTRIBUTE_TARGET_BLANK          0
#define ATTRIBUTE_TARGET_SELF           1
#define ATTRIBUTE_TYPE                  12
#define ATTRIBUTE_TYPE_REGULAR          0
#define ATTRIBUTE_TYPE_PASSWORD         1

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

};

struct attribute_mode
{

    int type;

};

struct attribute_onclick
{

    unsigned int type;
    char *data;

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

void attribute_data_create(struct attribute_data *attribute, char *content);
void attribute_data_destroy(struct attribute_data *attribute);
void attribute_onclick_create(struct attribute_onclick *attribute, unsigned int type, char *data);
void attribute_onclick_destroy(struct attribute_onclick *attribute);
void attribute_grid_create(struct attribute_grid *attribute, char *format);
void attribute_grid_destroy(struct attribute_grid *attribute);
void attribute_icon_create(struct attribute_icon *attribute, unsigned int type);
void attribute_icon_destroy(struct attribute_icon *attribute);
void attribute_id_create(struct attribute_id *attribute, char *name);
void attribute_id_destroy(struct attribute_id *attribute);
void attribute_in_create(struct attribute_in *attribute, char *name);
void attribute_in_destroy(struct attribute_in *attribute);
void attribute_label_create(struct attribute_label *attribute, char *content);
void attribute_label_destroy(struct attribute_label *attribute);
void attribute_link_create(struct attribute_link *attribute, char *url);
void attribute_link_destroy(struct attribute_link *attribute);
void attribute_mode_create(struct attribute_mode *attribute, unsigned int type);
void attribute_mode_destroy(struct attribute_mode *attribute);
void attribute_range_create(struct attribute_range *attribute, unsigned int min, unsigned int max);
void attribute_range_destroy(struct attribute_range *attribute);
void attribute_target_create(struct attribute_target *attribute, unsigned int type);
void attribute_target_destroy(struct attribute_target *attribute);
void attribute_type_create(struct attribute_type *attribute, unsigned int type);
void attribute_type_destroy(struct attribute_type *attribute);
