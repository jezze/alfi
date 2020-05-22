#define ATTRIBUTE_DATA                  1
#define ATTRIBUTE_GRID                  2
#define ATTRIBUTE_ICON                  3
#define ATTRIBUTE_ICON_NONE             0
#define ATTRIBUTE_ICON_CHECK            0xEC4B
#define ATTRIBUTE_ICON_CLOSE            0xEC4F
#define ATTRIBUTE_ICON_SEARCH           0xEC82
#define ATTRIBUTE_ID                    4
#define ATTRIBUTE_IN                    5
#define ATTRIBUTE_LABEL                 6
#define ATTRIBUTE_LINK                  7
#define ATTRIBUTE_MODE                  8
#define ATTRIBUTE_MODE_OFF              0
#define ATTRIBUTE_MODE_ON               1
#define ATTRIBUTE_MODE_DISABLED         2
#define ATTRIBUTE_RANGE                 9
#define ATTRIBUTE_TARGET                10
#define ATTRIBUTE_TARGET_BLANK          0
#define ATTRIBUTE_TARGET_SELF           1
#define ATTRIBUTE_TYPE                  11
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
