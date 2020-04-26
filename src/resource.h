#define RESOURCE_PAGESIZE               0x1000

struct resource
{

    struct list_item item;
    struct urlinfo urlinfo;
    void *data;
    unsigned int size;
    unsigned int count;
    unsigned int refcount;
    int index;
    unsigned char *img;
    int w;
    int h;
    int n;

};

unsigned int resource_load(struct resource *resource, unsigned int count, void *data);
unsigned int resource_iref(struct resource *resource);
unsigned int resource_dref(struct resource *resource);
void resource_init(struct resource *resource, char *url);
void resource_destroy(struct resource *resource);
