#include <stdlib.h>
#include "view.h"

static int clamp(int v, int min, int max)
{

    if (v < min)
        return min;

    if (v > max)
        return max;

    return v;

}

void view_init(struct view *view, int w, int h, unsigned int size)
{

    view->pagew = w;
    view->pageh = h;
    view->unitw = w / 28;

    if (view->unitw < 32)
        view->unitw = 32;

    view->unith = 40;
    view->marginw = 16;
    view->marginh = 16;
    view->padw = view->unitw;
    view->padh = view->unith;

    view_fontsize(view, size);

}

void view_fontsize(struct view *view, unsigned int size)
{

    view->fontsizesmall = 24 + 8 * size;
    view->fontsizemedium = 32 + 8 * size;
    view->fontsizelarge = 48 + 8 * size;
    view->fontsizexlarge = 96 + 8 * size;

}

void view_reset(struct view *view)
{

    view->scrollx = 0;
    view->scrolly = 0;

}

void view_scroll(struct view *view, int x, int y)
{

    view->scrollx += x * view->unitw;
    view->scrolly += y * view->unith;

}

void view_adjust(struct view *view, float w, float h)
{

    view->scrollx = (view->pagew < w) ? clamp(view->scrollx, view->pagew - w - view->padw * 2, 0) : view->pagew / 2 - (w + view->padw * 2) / 2;
    view->scrolly = (view->pageh < h) ? clamp(view->scrolly, view->pageh - h - view->padh * 2, 0) : 0;

}

