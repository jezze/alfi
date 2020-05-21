struct widget
{

    struct list_item item;
    struct widget_header header;
    union widget_payload payload;
    struct frame frame;

};

