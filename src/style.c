#include <stdlib.h>
#include <math.h>
#include "list.h"
#include "style.h"

static float flerp(float t, float c, float u)
{

    return c + roundf((t - c) * u);

}

void style_box_init(struct style_box *box, float x, float y, float w, float h, float r)
{

    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;
    box->r = r;

}

void style_box_clone(struct style_box *box, struct style_box *target)
{

    box->x = target->x;
    box->y = target->y;
    box->w = target->w;
    box->h = target->h;
    box->r = target->r;

}

void style_box_move(struct style_box *box, float x, float y)
{

    box->x = x;
    box->y = y;

}

void style_box_scale(struct style_box *box, float w, float h)
{

    box->w = w;
    box->h = h;

}

void style_box_translate(struct style_box *box, float x, float y)
{

    box->x += x;
    box->y += y;

}

void style_box_resize(struct style_box *box, float w, float h)
{

    box->w += w;
    box->h += h;

}

void style_box_pad(struct style_box *box, float px, float py)
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

float style_box_halign(struct style_box *box, float x, float w, int align)
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

float style_box_valign(struct style_box *box, float y, float h, int align)
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

unsigned int style_box_istouching(struct style_box *box, float x, float y)
{

    if (x < box->x || x >= box->x + box->w)
        return 0;

    if (y < box->y || y >= box->y + box->h)
        return 0;

    return 1;

}

void style_box_expand(struct style_box *box, struct style_box *child, float px, float py)
{

    if (box->w < child->w + px * 2)
        box->w = child->w + px * 2;

    if (box->h < child->h + py * 2)
        box->h = child->h + py * 2;

}

void style_box_lerp(struct style_box *box, float x, float y, float w, float h, float r, float u)
{

    box->x = flerp(x, box->x, u);
    box->y = flerp(y, box->y, u);
    box->w = flerp(w, box->w, u);
    box->h = flerp(h, box->h, u);
    box->r = flerp(r, box->r, u);

}

void style_box_tween(struct style_box *box, struct style_box *from, float u)
{

    style_box_lerp(box, from->x, from->y, from->w, from->h, from->r, u);

}

unsigned int style_box_compare(struct style_box *b1, struct style_box *b2)
{

    if (b1->x != b2->x)
        return 1;

    if (b1->y != b2->y)
        return 1;

    if (b1->w != b2->w)
        return 1;

    if (b1->h != b2->h)
        return 1;

    if (b1->r != b2->r)
        return 1;

    return 0;

}

void style_color_init(struct style_color *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{

    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;

}

void style_color_clone(struct style_color *color, struct style_color *target)
{

    color->r = target->r;
    color->g = target->g;
    color->b = target->b;
    color->a = target->a;

}

void style_color_lerp(struct style_color *color, float r, float g, float b, float a, float u)
{

    color->r = flerp(r, color->r, u);
    color->g = flerp(g, color->g, u);
    color->b = flerp(b, color->b, u);
    color->a = flerp(a, color->a, u);

}

void style_color_tween(struct style_color *color, struct style_color *from, float u)
{

    style_color_lerp(color, from->r, from->g, from->b, from->a, u);

}

unsigned int style_color_compare(struct style_color *c1, struct style_color *c2)
{

    if (c1->r != c2->r)
        return 1;

    if (c1->g != c2->g)
        return 1;

    if (c1->b != c2->b)
        return 1;

    if (c1->a != c2->a)
        return 1;

    return 0;

}

void style_font_init(struct style_font *font, int face, float size, int align)
{

    font->face = face;
    font->size = size;
    font->align = align;

}

void style_font_lerp(struct style_font *font, int face, float size, int align, float u)
{

    font->face = face;
    font->size = flerp(size, font->size, u);
    font->align = align;

}

void style_font_tween(struct style_font *font, struct style_font *from, float u)
{

    style_font_lerp(font, from->face, from->size, from->align, u);

}

unsigned int style_font_compare(struct style_font *f1, struct style_font *f2)
{

    if (f1->size != f2->size)
        return 1;

    return 0;

}

void style_tween(struct style *s1, struct style *s2, float u)
{

    style_box_tween(&s1->box, &s2->box, u);
    style_color_tween(&s1->color, &s2->color, u);
    style_font_tween(&s1->font, &s2->font, u);

}

void style_init(struct style *style)
{

    style_box_init(&style->box, 0, 0, 0, 0, 0);
    style_color_init(&style->color, 0, 0, 0, 0);
    style_font_init(&style->font, 0, 0, 0);

}

unsigned int style_compare(struct style *s1, struct style *s2)
{

    if (style_box_compare(&s1->box, &s2->box))
        return 1;

    if (style_color_compare(&s1->color, &s2->color))
        return 1;

    if (style_font_compare(&s1->font, &s2->font))
        return 1;

    return 0;

}

