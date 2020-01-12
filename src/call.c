#include <stdlib.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "widgets.h"
#include "view.h"
#include "call.h"

struct call
{

    unsigned int flags;
    void (*create)(struct widget *widget);
    void (*destroy)(struct widget *widget);
    int (*animate)(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u);
    void (*render)(struct widget *widget, struct frame *frame, struct view *view);
    unsigned int (*setstate)(struct widget *widget, unsigned int state);
    unsigned int (*getcursor)(struct widget *widget, struct frame *frame, int x, int y);

};

static struct call calls[64];

unsigned int call_checkflag(struct widget *widget, unsigned int flag)
{

    return calls[widget->header.type].flags & flag;

}

void call_create(struct widget *widget)
{

    calls[widget->header.type].create(widget);

}

void call_destroy(struct widget *widget)
{

    calls[widget->header.type].destroy(widget);

}

static void initframe(struct frame *frame, int x, int y, int w)
{

    unsigned int i;

    style_box_init(&frame->bounds, x, y, w, 0, 0);

    for (i = 0; i < 8; i++)
        style_init(&frame->styles[i]);

}

static void tweenframe(struct frame *frame, struct frame *keyframe, float u)
{

    unsigned int i;

    style_box_tween(&frame->bounds, &keyframe->bounds, u);

    for (i = 0; i < 8; i++)
        style_tween(&frame->styles[i], &keyframe->styles[i], u);

}

static unsigned int compareframe(struct frame *frame, struct frame *keyframe)
{

    unsigned int i;

    style_box_compare(&frame->bounds, &keyframe->bounds);
        return 1;

    for (i = 0; i < 8; i++)
    {

        if (style_compare(&frame->styles[i], &keyframe->styles[i]))
            return 1;

    }

    return 0;

}

int call_animate(struct widget *widget, int x, int y, int w, struct view *view, float u)
{

    struct frame *frame = &widget->frame;
    struct frame keyframe;

    initframe(&keyframe, x, y, w);

    keyframe.bounds.h = calls[widget->header.type].animate(widget, &keyframe, x, y, w, view, u);

    if (widget->header.type == ALFI_WIDGET_WINDOW)
        tweenframe(frame, &keyframe, 1.0);
    else
        tweenframe(frame, &keyframe, u);

    frame->animating = compareframe(frame, &keyframe);

    return frame->bounds.h;

}

void call_render(struct widget *widget, struct view *view)
{

    struct frame *frame = &widget->frame;

    calls[widget->header.type].render(widget, frame, view);

}

void call_setstate(struct widget *widget, unsigned int state)
{

    widget->state = calls[widget->header.type].setstate(widget, state);

}

unsigned int call_getcursor(struct widget *widget, int x, int y)
{

    struct frame *frame = &widget->frame;

    return calls[widget->header.type].getcursor(widget, frame, x, y);

}

void call_register(unsigned int type, unsigned int flags, void (*create)(struct widget *widget), void (*destroy)(struct widget *widget), int (*animate)(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u), void (*render)(struct widget *widget, struct frame *frame, struct view *view), unsigned int (*setstate)(struct widget *widget, unsigned int state), unsigned int (*getcursor)(struct widget *widget, struct frame *frame, int x, int y))
{

    calls[type].flags = flags;
    calls[type].create = create;
    calls[type].destroy = destroy;
    calls[type].animate = animate;
    calls[type].render = render;
    calls[type].setstate = setstate;
    calls[type].getcursor = getcursor;

}

