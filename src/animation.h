int animation_step(struct widget *widget, int x, int y, int w, struct view *view, float u);
void animation_render(struct widget *widget, struct view *view);
unsigned int animation_getcursor(struct widget *widget, int x, int y);
void animation_setupfonts(void);
void animation_settheme(unsigned int type);
void animation_setup(void);
