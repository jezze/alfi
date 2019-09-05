struct call
{

    unsigned int flags;
    void (*create)(struct alfi_widget *widget);
    void (*destroy)(struct alfi_widget *widget);
    void (*place)(struct alfi_widget *widget, float x, float y, float w, float h, float u);
    void (*render)(struct alfi_widget *widget);
    unsigned int (*setstate)(struct alfi_widget *widget, unsigned int state);
    void (*onclick)(struct alfi_widget *widget);
    unsigned int (*getcursor)(struct alfi_widget *widget, float x, float y);

};

unsigned int call_checkflag(struct alfi_widget *widget, unsigned int flag);
void call_create(struct alfi_widget *widget);
void call_destroy(struct alfi_widget *widget);
void call_place(struct alfi_widget *widget, float x, float y, float w, float h, float u);
void call_render(struct alfi_widget *widget);
void call_setstate(struct alfi_widget *widget, unsigned int state);
void call_onclick(struct alfi_widget *widget);
unsigned int call_getcursor(struct alfi_widget *widget, float x, float y);
void call_register(unsigned int type, unsigned int flags, void (*create)(struct alfi_widget *widget), void (*destroy)(struct alfi_widget *widget), void (*place)(struct alfi_widget *widget, float x, float y, float w, float h, float u), void (*render)(struct alfi_widget *widget), unsigned int (*setstate)(struct alfi_widget *widget, unsigned int state), void (*onclick)(struct alfi_widget *widget), unsigned int (*getcursor)(struct alfi_widget *widget, float x, float y));
