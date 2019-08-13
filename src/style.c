#include <stdlib.h>
#include <math.h>
#include "list.h"
#include "style.h"

float flerp(float t, float c, float u)
{

    return c + roundf((t - c) * u);

}

void box_init(struct alfi_box *box, float x, float y, float w, float h, float r)
{

    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;
    box->r = r;

}

void box_clone(struct alfi_box *box, struct alfi_box *target)
{

    box->x = target->x;
    box->y = target->y;
    box->w = target->w;
    box->h = target->h;
    box->r = target->r;

}

void box_move(struct alfi_box *box, float x, float y)
{

    box->x = x;
    box->y = y;

}

void box_scale(struct alfi_box *box, float w, float h)
{

    box->w = w;
    box->h = h;

}

void box_translate(struct alfi_box *box, float x, float y)
{

    box->x += x;
    box->y += y;

}

void box_resize(struct alfi_box *box, float w, float h)
{

    box->w += w;
    box->h += h;

}

void box_pad(struct alfi_box *box, float px, float py)
{

    box->x += px;
    box->y += py;
    box->w -= px * 2;
    box->h -= py * 2;

    if (box->w < 0)
        box->w = 0;

    if (box->h < 0)
        box->h = 0;

}

float box_halign(struct alfi_box *box, float x, float w, int align)
{

/*
    switch (align)
    {

    case ALFI_HALIGN_LEFT:
        return x;

    case ALFI_HALIGN_CENTER:
        return x + w * 0.5 - box->w * 0.5;

    case ALFI_HALIGN_RIGHT:
        return x + w - box->w;

    }
*/

    return 0;

}

float box_valign(struct alfi_box *box, float y, float h, int align)
{

/*
    switch (align)
    {

    case ALFI_VALIGN_TOP:
        return y;

    case ALFI_VALIGN_MIDDLE:
        return y + h * 0.5 - box->h * 0.5;

    case ALFI_VALIGN_BOTTOM:
        return y + h - box->h;

    }
*/

    return 0;

}

unsigned int box_istouching(struct alfi_box *box, float x, float y)
{

    if (x < box->x || x >= box->x + box->w)
        return 0;

    if (y < box->y || y >= box->y + box->h)
        return 0;

    return 1;

}

void box_expand2(struct alfi_box *box, float x, float y, float w, float h)
{

    if (box->x + box->w < x + w)
        box->w += (x + w) - (box->x + box->w);

    if (box->y + box->h < y + h)
        box->h += (y + h) - (box->y + box->h);

}

void box_lerp(struct alfi_box *box, float x, float y, float w, float h, float r, float u)
{

    box->x = flerp(x, box->x, u);
    box->y = flerp(y, box->y, u);
    box->w = flerp(w, box->w, u);
    box->h = flerp(h, box->h, u);
    box->r = flerp(r, box->r, u);

}

void box_lerpfrom(struct alfi_box *box, struct alfi_box *from, float u)
{

    box_lerp(box, from->x, from->y, from->w, from->h, from->r, u);

}

void color_init(struct alfi_color *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{

    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;

}

void color_clone(struct alfi_color *color, struct alfi_color *target)
{

    color->r = target->r;
    color->g = target->g;
    color->b = target->b;
    color->a = target->a;

}

void color_lerp(struct alfi_color *color, float r, float g, float b, float a, float u)
{

    color->r = flerp(r, color->r, u);
    color->g = flerp(g, color->g, u);
    color->b = flerp(b, color->b, u);
    color->a = flerp(a, color->a, u);

}

void color_lerpfrom(struct alfi_color *color, struct alfi_color *from, float u)
{

    color_lerp(color, from->r, from->g, from->b, from->a, u);

}

void font_init(struct alfi_font *font, int face, float size, float height, int align)
{

    font->face = face;
    font->size = size;
    font->height = height;
    font->align = align;

}

void font_lerp(struct alfi_font *font, int face, float size, float height, int align, float u)
{

    font->face = face;
    font->size = flerp(size, font->size, u);
    font->height = flerp(height, font->height, u);
    font->align = align;

}

void font_lerpfrom(struct alfi_font *font, struct alfi_font *from, float u)
{

    font_lerp(font, from->face, from->size, from->height, from->align, u);

}

