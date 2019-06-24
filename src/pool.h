struct alfi_widget *pool_nextingroup(struct alfi_widget *widget, unsigned int group);
struct alfi_widget *pool_nextingroupoftype(struct alfi_widget *widget, unsigned int group, unsigned int type);
struct alfi_widget *pool_findbyname(unsigned int group, char *name);
struct alfi_widget *pool_nextchild(struct alfi_widget *widget, struct alfi_widget *parent);
struct alfi_widget *pool_prevflag(struct alfi_widget *widget, unsigned int flag);
struct alfi_widget *pool_nextflag(struct alfi_widget *widget, unsigned int flag);
struct alfi_widget *pool_create(void);
void pool_destroy(struct alfi_widget *widget);
void pool_init(void);
