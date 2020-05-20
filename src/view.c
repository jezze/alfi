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

void view_init(struct view *view, int w, int h)
{

    view->pagew = w;
    view->pageh = h;
    view->unitw = (w > 1024) ? w / 32 : 32;
    view->unith = 40;
    view->marginw = 16;
    view->marginh = 16;
    view->scrollw = view->unitw * 28;
    view->scrollh = view->unith * 28;
    view->fontsizesmall = 24;
    view->fontsizemedium = 32;
    view->fontsizelarge = 48;
    view->fontsizexlarge = 96;

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

    view->scrollx = (view->pagew < w) ? clamp(view->scrollx, view->pagew - w, 0) : view->pagew / 2 - w / 2;
    view->scrolly = (view->pageh < h) ? clamp(view->scrolly, view->pageh - h, 0) : 0;

}

