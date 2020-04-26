#define RESOURCE_PAGESIZE               0x1000

struct resource
{

    struct list_item item;
    struct urlinfo urlinfo;
    void *data;
    unsigned int size;
    unsigned int count;
    unsigned int refcount;
    /* font */
    int index;

};

struct resource_image
{

    struct resource *base;
    unsigned char *img;
    int ref;
    int w;
    int h;
    int n;

};

void resource_save(struct resource *resource, unsigned int count, void *data);
unsigned int resource_load(struct resource *resource, unsigned int count, void *data);
unsigned int resource_iref(struct resource *resource);
unsigned int resource_dref(struct resource *resource);
void resource_init(struct resource *resource, char *url);
void resource_destroy(struct resource *resource);
