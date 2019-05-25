#include <math.h>
#include "list.h"
#include "box.h"

void box_init(struct alfi_box *box, float x, float y, float w, float h)
{

    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;

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

void box_expand(struct alfi_box *box, float x, float y, float w, float h)
{

    if (box->x + box->w < x + w)
        box->w += (x + w) - (box->x + box->w);

    if (box->y + box->h < y + h)
        box->h += (y + h) - (box->y + box->h);

}

void box_lerp(struct alfi_box *box, float x, float y, float w, float h, float u)
{

    box->x += (int)((x - box->x) * u);
    box->y += (int)((y - box->y) * u);
    box->w += (int)((w - box->w) * u);
    box->h += (int)((h - box->h) * u);

}

void color_init(struct alfi_color *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{

    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;

}

void color_lerp(struct alfi_color *color, float r, float g, float b, float a, float u)
{

    color->r += (char)((r - color->r) * u);
    color->g += (char)((g - color->g) * u);
    color->b += (char)((b - color->b) * u);
    color->a += (char)((a - color->a) * u);

}

void font_init(struct alfi_font *font, int face, float size, int align)
{

    font->face = face;
    font->size = size;
    font->align = align;

}

void font_lerp(struct alfi_font *font, float size, float u)
{

    font->size += (int)((size - font->size) * u);

}

