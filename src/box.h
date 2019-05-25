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
    int align;

};

void box_init(struct alfi_box *box, float x, float y, float w, float h);
void box_move(struct alfi_box *box, float x, float y);
void box_scale(struct alfi_box *box, float w, float h);
float box_halign(struct alfi_box *box, float x, float w, int align);
float box_valign(struct alfi_box *box, float y, float h, int align);
unsigned int box_istouching(struct alfi_box *box, float x, float y);
void box_expand(struct alfi_box *box, float x, float y, float w, float h);
void box_lerp(struct alfi_box *box, float x, float y, float w, float h, float u);
void color_init(struct alfi_color *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void color_lerp(struct alfi_color *color, float r, float g, float b, float a, float u);
void font_init(struct alfi_font *font, int face, float size, int align);
void font_lerp(struct alfi_font *font, float size, float u);
