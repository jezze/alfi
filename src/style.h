enum style_align
{

    STYLE_ALIGN_LEFT               = 1 << 0,
    STYLE_ALIGN_CENTER             = 1 << 1,
    STYLE_ALIGN_RIGHT              = 1 << 2,
    STYLE_ALIGN_TOP                = 1 << 3,
    STYLE_ALIGN_MIDDLE             = 1 << 4,
    STYLE_ALIGN_BOTTOM             = 1 << 5,
    STYLE_ALIGN_BASELINE           = 1 << 6

};

struct style_box
{

    float x;
    float y;
    float w;
    float h;
    float r;

};

struct style_color
{

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

};

struct style_font
{

    int face;
    float size;
    unsigned int align;

};

struct style
{

    struct style_box box;
    struct style_color color;
    struct style_font font;

};

struct frame
{

    struct style_box bounds;
    struct style styles[8];
    unsigned int nstyles;
    unsigned int animating;

};

void style_box_init(struct style_box *box, float x, float y, float w, float h, float r);
void style_box_clone(struct style_box *box, struct style_box *target);
void style_box_move(struct style_box *box, float x, float y);
void style_box_scale(struct style_box *box, float w, float h);
void style_box_translate(struct style_box *box, float x, float y);
void style_box_resize(struct style_box *box, float w, float h);
void style_box_shrink(struct style_box *box, float px, float py);
float style_box_halign(struct style_box *box, float x, float w, int align);
float style_box_valign(struct style_box *box, float y, float h, int align);
unsigned int style_box_istouching(struct style_box *box, float x, float y);
void style_box_expand(struct style_box *box, struct style_box *child, float px, float py);
void style_box_lerp(struct style_box *box, float x, float y, float w, float h, float r, float u);
void style_box_tween(struct style_box *box, struct style_box *from, float u);
unsigned int style_box_compare(struct style_box *b1, struct style_box *b2);
void style_color_init(struct style_color *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void style_color_clone(struct style_color *color, struct style_color *target);
void style_color_lerp(struct style_color *color, float r, float g, float b, float a, float u);
void style_color_tween(struct style_color *color, struct style_color *from, float u);
unsigned int style_color_compare(struct style_color *c1, struct style_color *c2);
void style_font_init(struct style_font *font, int face, float size, int align);
void style_font_lerp(struct style_font *font, int face, float size, int align, float u);
void style_font_tween(struct style_font *font, struct style_font *from, float u);
unsigned int style_font_compare(struct style_font *f1, struct style_font *f2);
void style_tween(struct style *s1, struct style *s2, float u);
void style_init(struct style *style);
unsigned int style_compare(struct style *s1, struct style *s2);
