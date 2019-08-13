#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "widgets.h"

struct call
{

    unsigned int flags;
    void (*init)(struct alfi_widget *widget);
    void (*destroy)(struct alfi_widget *widget);
    void (*place)(struct alfi_widget *widget, float x, float y, float w, float h, float u);
    void (*render)(struct alfi_widget *widget);
    unsigned int (*setstate)(struct alfi_widget *widget, unsigned int state);
    void (*onclick)(struct alfi_widget *widget);
    unsigned int (*getcursor)(struct alfi_widget *widget, float x, float y);

};

static struct call calls[64];

unsigned int call_checkflag(struct alfi_widget *widget, unsigned int flag)
{

    return calls[widget->type].flags & flag;

}

void call_init(struct alfi_widget *widget)
{

    if (calls[widget->type].init)
        calls[widget->type].init(widget);

}

void call_destroy(struct alfi_widget *widget)
{

    if (calls[widget->type].destroy)
        calls[widget->type].destroy(widget);

}

void call_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    calls[widget->type].place(widget, x, y, w, h, u);

}

void call_render(struct alfi_widget *widget)
{

    calls[widget->type].render(widget);

}

void call_setstate(struct alfi_widget *widget, unsigned int state)
{

    widget->state = calls[widget->type].setstate(widget, state);

}

void call_onclick(struct alfi_widget *widget)
{

    calls[widget->type].onclick(widget);

}

unsigned int call_getcursor(struct alfi_widget *widget, float x, float y)
{

    return calls[widget->type].getcursor(widget, x, y);

}

void call_register(unsigned int type, unsigned int flags, void (*init)(struct alfi_widget *widget), void (*destroy)(struct alfi_widget *widget), void (*place)(struct alfi_widget *widget, float x, float y, float w, float h, float u), void (*render)(struct alfi_widget *widget), unsigned int (*setstate)(struct alfi_widget *widget, unsigned int state), void (*onclick)(struct alfi_widget *widget), unsigned int (*getcursor)(struct alfi_widget *widget, float x, float y))
{

    calls[type].flags = flags;
    calls[type].init = init;
    calls[type].destroy = destroy;
    calls[type].place = place;
    calls[type].render = render;
    calls[type].setstate = setstate;
    calls[type].onclick = onclick;
    calls[type].getcursor = getcursor;

}

