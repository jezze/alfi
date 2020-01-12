unsigned int call_checkflag(struct widget *widget, unsigned int flag);
void call_create(struct widget *widget);
void call_destroy(struct widget *widget);
int call_animate(struct widget *widget, int x, int y, int w, struct view *view, float u);
void call_render(struct widget *widget, struct view *view);
void call_setstate(struct widget *widget, unsigned int state);
unsigned int call_getcursor(struct widget *widget, int x, int y);
void call_register(unsigned int type, unsigned int flags, void (*create)(struct widget *widget), void (*destroy)(struct widget *widget), int (*animate)(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u), void (*render)(struct widget *widget, struct frame *frame, struct view *view), unsigned int (*setstate)(struct widget *widget, unsigned int state), unsigned int (*getcursor)(struct widget *widget, struct frame *frame, int x, int y));
