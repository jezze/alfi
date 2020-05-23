#define ANIMATION_CURSOR_ARROW          0
#define ANIMATION_CURSOR_IBEAM          1
#define ANIMATION_CURSOR_HAND           2
#define ANIMATION_THEME_LIGHT           0
#define ANIMATION_THEME_DARK            1

void animation_updateframe(unsigned int type, struct frame *frame, struct frame *keyframe, float u);
void animation_initframe(struct frame *frame, int x, int y, int w, int h);
void animation_step(struct widget *widget, struct frame *frame, struct view *view, float u);
void animation_render(struct widget *widget, struct view *view);
unsigned int animation_getcursor(struct widget *widget, int x, int y);
void animation_setupfonts(void);
void animation_settheme(unsigned int type);
