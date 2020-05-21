#define ENTITY_FLAG_NONE                0
#define ENTITY_FLAG_FOCUSABLE           1

struct widget
{

    struct list_item item;
    struct widget_header header;
    union widget_payload payload;
    struct frame frame;

};

void entity_create(struct widget *widget, unsigned int type, char *id, char *in);
void entity_destroy(struct widget *widget);
void entity_setstate(struct widget *widget, unsigned int state);
