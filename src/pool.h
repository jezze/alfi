struct widget *pool_widget_prev(struct widget *widget);
struct widget *pool_widget_next(struct widget *widget);
struct widget *pool_widget_find(char *name);
struct widget *pool_widget_nextchild(struct widget *widget, struct widget *parent);
struct widget *pool_widget_create(void);
struct widget *pool_widget_destroy(struct widget *widget);
struct resource *pool_resource_find(char *url);
struct resource *pool_resource_create(void);
struct resource *pool_resource_destroy(struct resource *resource);
char *pool_allocate(unsigned int type, char *string, unsigned int size, unsigned int count, char *content);
char *pool_string_create(unsigned int type, char *string, char *content);
char *pool_string_destroy(unsigned int type, char *string);
void pool_setup(void);
