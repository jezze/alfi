#define RESOURCE_PAGESIZE               0x1000

struct resource
{

    struct list_item item;
    char url[URL_SIZE];
    void *data;
    unsigned int size;
    unsigned int count;
    unsigned int refcount;
    int index;
    int w;
    int h;
    int n;

};

void resource_load(struct resource *resource, unsigned int count, void *data);
void resource_unload(struct resource *resource);
unsigned int resource_iref(struct resource *resource);
unsigned int resource_dref(struct resource *resource);
void resource_init(struct resource *resource, char *url);
void resource_destroy(struct resource *resource);
