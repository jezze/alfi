struct alfi_box
{

    float x;
    float y;
    float w;
    float h;

};

struct alfi_color
{

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

};

struct alfi_font
{

    int face;
    float size;
    float height;
    int align;

};

struct alfi_style
{

    struct alfi_box box;
    struct alfi_color color;
    struct alfi_font font;
    float radius;

};

float flerp(float t, float c, float u);
void box_init(struct alfi_box *box, float x, float y, float w, float h);
void box_clone(struct alfi_box *box, struct alfi_box *target);
void box_move(struct alfi_box *box, float x, float y);
void box_scale(struct alfi_box *box, float w, float h);
void box_translate(struct alfi_box *box, float x, float y);
void box_pad(struct alfi_box *box, float px, float py);
float box_halign(struct alfi_box *box, float x, float w, int align);
float box_valign(struct alfi_box *box, float y, float h, int align);
unsigned int box_istouching(struct alfi_box *box, float x, float y);
void box_expand(struct alfi_box *box, float x, float y, float w, float h);
void box_lerp(struct alfi_box *box, float x, float y, float w, float h, float u);
void box_lerpfrom(struct alfi_box *box, struct alfi_box *from, float u);
void color_init(struct alfi_color *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void color_clone(struct alfi_color *color, struct alfi_color *target);
void color_lerp(struct alfi_color *color, float r, float g, float b, float a, float u);
void color_lerpfrom(struct alfi_color *color, struct alfi_color *from, float u);
void font_init(struct alfi_font *font, int face, float size, float height, int align);
void font_lerp(struct alfi_font *font, int face, float size, float height, int align, float u);
void font_lerpfrom(struct alfi_font *font, struct alfi_font *from, float u);
