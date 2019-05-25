#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <nanovg.h>
#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg_gl.h>
#include "list.h"
#include "box.h"
#include "widgets.h"
#include "parser.h"

#define NWIDGETS                        512
#define STRINGTABLESIZE                 8192

static struct alfi_widget widgets[NWIDGETS];
static char strings[STRINGTABLESIZE];
static struct parser parser;
static struct list freelist;
static struct list usedlist;
static float marginx = 8.0;
static float marginy = 8.0;
static float gridx = 32.0;
static float gridy = 32.0;
static int fontface_regular;
static int fontface_bold;
static int fontface_icon;
static float fontsize_small;
static float fontsize_medium;
static float fontsize_large;
static float fontsize_xlarge;
GLFWcursor *cursor_arrow;
GLFWcursor *cursor_ibeam;
GLFWcursor *cursor_hand;
static double mx;
static double my;
static int scrollx;
static int scrolly;
static int winw;
static int winh;
static int fbw;
static int fbh;
static struct alfi_widget *hover;
static NVGcontext *ctx;

struct call
{

    unsigned int flags;
    void (*init)(struct alfi_widget *widget);
    void (*place)(struct alfi_widget *widget, float x, float y, float w, float h);
    void (*animate)(struct alfi_widget *widget, float u);
    void (*render)(struct alfi_widget *widget, double dt);
    unsigned int (*signal)(struct alfi_widget *widget, unsigned int state);

};

struct call calls[64];

static struct alfi_widget *nextwidget(struct list *list, struct alfi_widget *widget)
{

    struct list_item *current = (widget) ? widget->item.next : list->head;

    return (current) ? current->data : 0;

}

static struct alfi_widget *nexttype(struct list *list, struct alfi_widget *widget, unsigned int type)
{

    while ((widget = nextwidget(list, widget)))
    {

        if (widget->type == type)
            return widget;

    }

    return 0;

}

static struct alfi_widget *nextgroup(struct list *list, struct alfi_widget *widget, unsigned int group)
{

    while ((widget = nextwidget(list, widget)))
    {

        if (widget->group == group)
            return widget;

    }

    return 0;

}

static struct alfi_widget *findroot(struct list *list, float x, float y)
{

    struct alfi_widget *root = 0;

    while ((root = nexttype(list, root, ALFI_WIDGET_WINDOW)))
    {

        if (box_istouching(&root->bb, x, y))
            return root;

    }

    return 0;

}

static struct alfi_widget *findwidgetbyname(struct list *list, unsigned int group, char *name)
{

    struct alfi_widget *widget = 0;

    while ((widget = nextgroup(list, widget, group)))
    {

        if (!widget->id.name)
            continue;

        if (!strcmp(widget->id.name, name))
            return widget;

    }

    return 0;

}

static struct alfi_widget *nextchild(struct list *list, struct alfi_widget *widget, struct alfi_widget *parent)
{

    while ((widget = nextgroup(list, widget, parent->group)))
    {

        if (findwidgetbyname(list, widget->group, widget->in.name) == parent)
            return widget;

    }

    return 0;

}

static struct alfi_widget *findwidgetbyposition(struct alfi_widget *parent, float x, float y)
{

    struct alfi_widget *child = 0;

    while ((child = nextchild(&usedlist, child, parent)))
    {

        if (box_istouching(&child->bb, x, y))
            return findwidgetbyposition(child, x, y);

    }

    return parent;

}

static void widget_button_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_button *current = &widget->animcurrent.button;
    struct alfi_render_button *target = &widget->animtarget.button;

    box_lerp(&current->background.box, target->background.box.x, target->background.box.y, target->background.box.w, target->background.box.h, u);
    color_lerp(&current->background.color, target->background.color.r, target->background.color.g, target->background.color.b, target->background.color.a, u);
    box_lerp(&current->label.box, target->label.box.x, target->label.box.y, target->label.box.w, target->label.box.h, u);
    color_lerp(&current->label.color, target->label.color.r, target->label.color.g, target->label.color.b, target->label.color.a, u);
    font_lerp(&current->label.font, target->label.font.size, u);

}

static void widget_button_init(struct alfi_widget *widget)
{

    if (!widget->data.button.label.content)
        widget->data.button.label.content = "undefined";

}

static void widget_button_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_button *target = &widget->animtarget.button;
    float selfw = gridx * 6;
    float selfh = gridy * 2;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, selfw, selfh);

    switch (widget->state)
    {

    case ALFI_STATE_HOVER:
        box_init(&target->background.box, widget->bb.x + marginx, widget->bb.y + marginy, widget->bb.w - marginx * 2, widget->bb.h - marginy * 2);
        color_init(&target->background.color, 64, 160, 160, 255);
        box_init(&target->label.box, widget->bb.x + widget->bb.w * 0.5, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 255, 255, 255, 255);
        font_init(&target->label.font, fontface_bold, fontsize_medium, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        break;

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->background.box, widget->bb.x + marginx, widget->bb.y + marginy, widget->bb.w - marginx * 2, widget->bb.h - marginy * 2);
        color_init(&target->background.color, 96, 192, 192, 255);
        box_init(&target->label.box, widget->bb.x + widget->bb.w * 0.5, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 255, 255, 255, 255);
        font_init(&target->label.font, fontface_bold, fontsize_medium, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        break;

    }

}

static void widget_button_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_button *current = &widget->animcurrent.button;
    float radius = 8.0;

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, current->background.box.x, current->background.box.y, current->background.box.w, current->background.box.h, radius);
    nvgFillColor(ctx, nvgRGBA(current->background.color.r, current->background.color.g, current->background.color.b, current->background.color.a));
    nvgFill(ctx);

    if (widget->data.button.label.content)
    {

        nvgFillColor(ctx, nvgRGBA(current->label.color.r, current->label.color.g, current->label.color.b, current->label.color.a));
        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgText(ctx, current->label.box.x, current->label.box.y, widget->data.button.label.content, 0);

    }

}

static unsigned int widget_button_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_choice_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, h);

}

static void widget_divider_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_divider *current = &widget->animcurrent.divider;
    struct alfi_render_divider *target = &widget->animtarget.divider;

    box_lerp(&current->border.box, target->border.box.x, target->border.box.y, target->border.box.w, target->border.box.h, u);
    color_lerp(&current->border.color, target->border.color.r, target->border.color.g, target->border.color.b, target->border.color.a, u);
    box_lerp(&current->label.box, target->label.box.x, target->label.box.y, target->label.box.w, target->label.box.h, u);
    color_lerp(&current->label.color, target->label.color.r, target->label.color.g, target->label.color.b, target->label.color.a, u);
    font_lerp(&current->label.font, target->label.font.size, u);

}

static void widget_divider_init(struct alfi_widget *widget)
{

}

static void widget_divider_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_divider *target = &widget->animtarget.divider;
    float selfh = gridy;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, selfh);

    switch (widget->state)
    {

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->border.box, widget->bb.x + marginx, widget->bb.y + widget->bb.h * 0.5 - 1, widget->bb.w - marginx * 2, 2);
        color_init(&target->border.color, 192, 192, 192, 255);
        box_init(&target->label.box, widget->bb.x + widget->bb.w * 0.5, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 128, 128, 128, 255);
        font_init(&target->label.font, fontface_regular, fontsize_small, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        break;

    }

}

static void widget_divider_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_divider *current = &widget->animcurrent.divider;

    nvgBeginPath(ctx);
    nvgRect(ctx, current->border.box.x, current->border.box.y, current->border.box.w, current->border.box.h);
    nvgFillColor(ctx, nvgRGBA(current->border.color.r, current->border.color.g, current->border.color.b, current->border.color.a));
    nvgFill(ctx);

    if (widget->data.divider.label.content)
    {

        float bounds[4];

        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgTextBounds(ctx, 0.0, 0.0, widget->data.divider.label.content, 0, bounds);
        nvgBeginPath(ctx);
        nvgRect(ctx, current->label.box.x - bounds[2] - marginx, current->label.box.y - marginy, bounds[2] * 2 + marginx * 2, marginy * 2);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgFill(ctx);
        nvgFillColor(ctx, nvgRGBA(current->label.color.r, current->label.color.g, current->label.color.b, current->label.color.a));
        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgText(ctx, current->label.box.x, current->label.box.y, widget->data.divider.label.content, 0);

    }

}

static unsigned int widget_divider_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_field_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_field *current = &widget->animcurrent.field;
    struct alfi_render_field *target = &widget->animtarget.field;

    box_lerp(&current->border.box, target->border.box.x, target->border.box.y, target->border.box.w, target->border.box.h, u);
    color_lerp(&current->border.color, target->border.color.r, target->border.color.g, target->border.color.b, target->border.color.a, u);
    box_lerp(&current->label.box, target->label.box.x, target->label.box.y, target->label.box.w, target->label.box.h, u);
    color_lerp(&current->label.color, target->label.color.r, target->label.color.g, target->label.color.b, target->label.color.a, u);
    font_lerp(&current->label.font, target->label.font.size, u);

}

static void widget_field_init(struct alfi_widget *widget)
{

}

static void widget_field_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_field *target = &widget->animtarget.field;
    float selfw = gridx * 12;
    float selfh = gridy * 2;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, selfw, selfh);

    switch (widget->state)
    {

    case ALFI_STATE_HOVER:
        box_init(&target->border.box, widget->bb.x + marginx, widget->bb.y + marginy, widget->bb.w - marginx * 2, widget->bb.h - marginy * 2);
        color_init(&target->border.color, 128, 128, 128, 255);
        box_init(&target->label.box, widget->bb.x + gridx, widget->bb.y + marginy * 0.5 + 2, 0, 0);
        color_init(&target->label.color, 128, 128, 128, 255);
        font_init(&target->label.font, fontface_regular, fontsize_small, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        break;

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->border.box, widget->bb.x + marginx, widget->bb.y + marginy, widget->bb.w - marginx * 2, widget->bb.h - marginy * 2);
        color_init(&target->border.color, 192, 192, 192, 255);
        box_init(&target->label.box, widget->bb.x + gridx, widget->bb.y + gridy, 0, 0);
        color_init(&target->label.color, 192, 192, 192, 255);
        font_init(&target->label.font, fontface_regular, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        break;

    }

}

static void widget_field_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_field *current = &widget->animcurrent.field;
    float radius = 8.0;

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, current->border.box.x, current->border.box.y, current->border.box.w, current->border.box.h, radius);
    nvgFillColor(ctx, nvgRGBA(current->border.color.r, current->border.color.g, current->border.color.b, current->border.color.a));
    nvgFill(ctx);
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, current->border.box.x + 2, current->border.box.y + 2, current->border.box.w - 4, current->border.box.h - 4, radius);
    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
    nvgFill(ctx);

    if (widget->data.field.label.content)
    {

        float bounds[4];

        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgTextBounds(ctx, 0.0, 0.0, widget->data.field.label.content, 0, bounds);
        nvgBeginPath(ctx);
        nvgRect(ctx, current->label.box.x - marginx, current->label.box.y - marginy, bounds[2] + marginx * 2, marginy * 2);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgFill(ctx);
        nvgFillColor(ctx, nvgRGBA(current->label.color.r, current->label.color.g, current->label.color.b, current->label.color.a));
        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgText(ctx, current->label.box.x, current->label.box.y, widget->data.field.label.content, 0);

    }

}

static unsigned int widget_field_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_header_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_header *current = &widget->animcurrent.header;
    struct alfi_render_header *target = &widget->animtarget.header;

    box_lerp(&current->label.box, target->label.box.x, target->label.box.y, target->label.box.w, target->label.box.h, u);
    color_lerp(&current->label.color, target->label.color.r, target->label.color.g, target->label.color.b, target->label.color.a, u);
    font_lerp(&current->label.font, target->label.font.size, u);

}

static void widget_header_init(struct alfi_widget *widget)
{

    if (!widget->data.header.label.content)
        widget->data.header.label.content = "undefined";

}

static void widget_header_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_header *target = &widget->animtarget.header;
    float selfh = gridy * 2;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, selfh);

    switch (widget->state)
    {

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->label.box, widget->bb.x + marginx, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 64, 64, 64, 255);
        font_init(&target->label.font, fontface_bold, fontsize_xlarge, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        break;

    }

}

static void widget_header_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_header *current = &widget->animcurrent.header;

    if (widget->data.header.label.content)
    {

        nvgFillColor(ctx, nvgRGBA(current->label.color.r, current->label.color.g, current->label.color.b, current->label.color.a));
        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgText(ctx, current->label.box.x, current->label.box.y, widget->data.header.label.content, 0);

    }

}

static unsigned int widget_header_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_hstack_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_widget *child = 0;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, h);

    while ((child = nextchild(&usedlist, child, widget)))
    {

        calls[child->type].place(child, widget->bb.x + widget->cx, widget->bb.y, 0, widget->bb.h);
        box_expand(&widget->bb, child->bb.x, child->bb.y, child->bb.w, child->bb.h);

        widget->cx += child->bb.w;

    }

}

static void widget_image_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_image *current = &widget->animcurrent.image;
    struct alfi_render_image *target = &widget->animtarget.image;

    box_lerp(&current->border.box, target->border.box.x, target->border.box.y, target->border.box.w, target->border.box.h, u);
    color_lerp(&current->border.color, target->border.color.r, target->border.color.g, target->border.color.b, target->border.color.a, u);

}

static void widget_image_init(struct alfi_widget *widget)
{

    widget->data.image.ref = nvgCreateImage(ctx, widget->data.image.link.url, 0);

    nvgImageSize(ctx, widget->data.image.ref, &widget->data.image.w, &widget->data.image.h);

}

static void widget_image_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_image *target = &widget->animtarget.image;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, widget->data.image.w + marginx * 2, widget->data.image.h + marginy * 2);

    switch (widget->state)
    {

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->border.box, widget->bb.x + marginx, widget->bb.y + marginy, widget->bb.w - marginx * 2, widget->bb.h - marginy * 2);
        color_init(&target->border.color, 255, 255, 255, 255);

        break;

    }

}

static void widget_image_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_image *current = &widget->animcurrent.image;
    NVGpaint paint = nvgImagePattern(ctx, current->border.box.x, current->border.box.y, current->border.box.w, current->border.box.h, 0.0, widget->data.image.ref, 1.0);

    nvgBeginPath(ctx);
    nvgRect(ctx, current->border.box.x, current->border.box.y, current->border.box.w, current->border.box.h);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);

}

static unsigned int widget_image_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_list_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_list *current = &widget->animcurrent.list;
    struct alfi_render_list *target = &widget->animtarget.list;

    box_lerp(&current->dot.box, target->dot.box.x, target->dot.box.y, target->dot.box.w, target->dot.box.h, u);
    color_lerp(&current->dot.color, target->dot.color.r, target->dot.color.g, target->dot.color.b, target->dot.color.a, u);

}

static void widget_list_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_widget *child = 0;
    struct alfi_render_list *target = &widget->animtarget.list;
    float pointsize = 3.0;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, h);

    while ((child = nextchild(&usedlist, child, widget)))
    {

        calls[child->type].place(child, widget->bb.x + gridx, widget->bb.y + widget->cy, widget->bb.w - gridx, 0);
        box_expand(&widget->bb, child->bb.x, child->bb.y, child->bb.w, child->bb.h);

        widget->cy += child->bb.h;

    }

    switch (widget->state)
    {

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->dot.box, widget->bb.x - marginx, widget->bb.y + gridy * 0.5 + pointsize * 0.5, pointsize, pointsize);
        color_init(&target->dot.color, 64, 64, 64, 255);

        break;

    }

}

static void widget_list_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_list *current = &widget->animcurrent.list;
    struct alfi_widget *child = 0;

    while ((child = nextchild(&usedlist, child, widget)))
    {

        if (!(calls[child->type].flags & ALFI_FLAG_ITEM))
            continue;

        nvgBeginPath(ctx);
        nvgCircle(ctx, current->dot.box.x + child->bb.x - widget->bb.x, current->dot.box.y + child->bb.y - widget->bb.y, current->dot.box.w);
        nvgFillColor(ctx, nvgRGBA(current->dot.color.r, current->dot.color.g, current->dot.color.b, current->dot.color.a));
        nvgFill(ctx);

    }

}

static unsigned int widget_list_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_select_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_select *current = &widget->animcurrent.select;
    struct alfi_render_select *target = &widget->animtarget.select;

    box_lerp(&current->background.box, target->background.box.x, target->background.box.y, target->background.box.w, target->background.box.h, u);
    color_lerp(&current->background.color, target->background.color.r, target->background.color.g, target->background.color.b, target->background.color.a, u);
    box_lerp(&current->label.box, target->label.box.x, target->label.box.y, target->label.box.w, target->label.box.h, u);
    color_lerp(&current->label.color, target->label.color.r, target->label.color.g, target->label.color.b, target->label.color.a, u);
    font_lerp(&current->label.font, target->label.font.size, u);

}

static void widget_select_init(struct alfi_widget *widget)
{

    if (!widget->data.select.label.content)
        widget->data.select.label.content = "undefined";

}

static void widget_select_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_select *target = &widget->animtarget.select;
    struct alfi_widget *child = 0;
    float selfw = gridx * 12;
    float selfh = gridy * 2;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, selfw, selfh);

    while ((child = nextchild(&usedlist, child, widget)))
    {

        calls[child->type].place(child, widget->bb.x, widget->bb.y, 0, 0);
        box_expand(&widget->bb, child->bb.x, child->bb.y, child->bb.w, child->bb.h);

    }

    switch (widget->state)
    {

    case ALFI_STATE_HOVER:
        box_init(&target->background.box, widget->bb.x + marginx, widget->bb.y + marginy, widget->bb.w - marginx * 2, widget->bb.h - marginy * 2);
        color_init(&target->background.color, 128, 128, 128, 255);
        box_init(&target->label.box, widget->bb.x + gridx, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 128, 128, 128, 255);
        font_init(&target->label.font, fontface_regular, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        break;

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->background.box, widget->bb.x + marginx, widget->bb.y + marginy, widget->bb.w - marginx * 2, widget->bb.h - marginy * 2);
        color_init(&target->background.color, 192, 192, 192, 255);
        box_init(&target->label.box, widget->bb.x + gridx, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 192, 192, 192, 255);
        font_init(&target->label.font, fontface_regular, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        break;

    }

}

static void widget_select_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_select *current = &widget->animcurrent.select;
    float radius = 8.0;

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, current->background.box.x, current->background.box.y, current->background.box.w, current->background.box.h, radius);
    nvgFillColor(ctx, nvgRGBA(current->background.color.r, current->background.color.g, current->background.color.b, current->background.color.a));
    nvgFill(ctx);
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, current->background.box.x + 2, current->background.box.y + 2, current->background.box.w - 4, current->background.box.h - 4, radius);
    nvgFillColor(ctx, nvgRGBA(240, 240, 240, 255));
    nvgFill(ctx);

    if (widget->data.select.label.content)
    {

        nvgFillColor(ctx, nvgRGBA(current->label.color.r, current->label.color.g, current->label.color.b, current->label.color.a));
        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgText(ctx, current->label.box.x, current->label.box.y, widget->data.select.label.content, 0);

    }

}

static unsigned int widget_select_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_subheader_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_subheader *current = &widget->animcurrent.subheader;
    struct alfi_render_subheader *target = &widget->animtarget.subheader;

    box_lerp(&current->label.box, target->label.box.x, target->label.box.y, target->label.box.w, target->label.box.h, u);
    color_lerp(&current->label.color, target->label.color.r, target->label.color.g, target->label.color.b, target->label.color.a, u);
    font_lerp(&current->label.font, target->label.font.size, u);

}

static void widget_subheader_init(struct alfi_widget *widget)
{

    if (!widget->data.subheader.label.content)
        widget->data.subheader.label.content = "undefined";

}

static void widget_subheader_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_subheader *target = &widget->animtarget.subheader;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, gridy * 2);

    switch (widget->state)
    {

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->label.box, widget->bb.x + marginx, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 64, 64, 64, 255);
        font_init(&target->label.font, fontface_bold, fontsize_large, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        break;

    }

}

static void widget_subheader_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_subheader *current = &widget->animcurrent.subheader;

    if (widget->data.subheader.label.content)
    {

        nvgFillColor(ctx, nvgRGBA(current->label.color.r, current->label.color.g, current->label.color.b, current->label.color.a));
        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgText(ctx, current->label.box.x, current->label.box.y, widget->data.subheader.label.content, 0);

    }

}

static unsigned int widget_subheader_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_table_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_widget *child = 0;
    float childw = gridx * widget->data.table.grid.clength;
    float childh = gridy * widget->data.table.grid.rlength;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, h);

    while ((child = nextchild(&usedlist, child, widget)))
    {

        calls[child->type].place(child, widget->bb.x + widget->cx, widget->bb.y + widget->cy, childw, childh);
        box_expand(&widget->bb, child->bb.x, child->bb.y, child->bb.w, child->bb.h);

        widget->cx += childw;

        if (widget->cx >= gridx * widget->data.table.grid.csize)
        {

            widget->cx = 0;

            if (widget->data.table.grid.rlength)
                widget->cy += childh;
            else
                widget->cy = widget->bb.h;

        }

    }

}

static void widget_text_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_text *current = &widget->animcurrent.text;
    struct alfi_render_text *target = &widget->animtarget.text;

    box_lerp(&current->label.box, target->label.box.x, target->label.box.y, target->label.box.w, target->label.box.h, u);
    color_lerp(&current->label.color, target->label.color.r, target->label.color.g, target->label.color.b, target->label.color.a, u);
    font_lerp(&current->label.font, target->label.font.size, u);

}

static void widget_text_init(struct alfi_widget *widget)
{

    if (!widget->data.text.label.content)
        widget->data.text.label.content = "undefined";

}

static void widget_text_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_text *target = &widget->animtarget.text;
    float selfh = gridy;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, selfh);

    switch (widget->state)
    {

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->label.box, widget->bb.x + marginx, widget->bb.y + widget->bb.h * 0.5, 0, 0);
        color_init(&target->label.color, 128, 128, 128, 255);
        font_init(&target->label.font, fontface_regular, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        break;

    }

}

static void widget_text_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_text *current = &widget->animcurrent.text;

    if (widget->data.text.label.content)
    {

        nvgFillColor(ctx, nvgRGBA(current->label.color.r, current->label.color.g, current->label.color.b, current->label.color.a));
        nvgFontFaceId(ctx, current->label.font.face);
        nvgFontSize(ctx, current->label.font.size);
        nvgTextAlign(ctx, current->label.font.align);
        nvgText(ctx, current->label.box.x, current->label.box.y, widget->data.text.label.content, 0);

    }

}

static unsigned int widget_text_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void widget_vstack_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_widget *child = 0;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, w, h);

    while ((child = nextchild(&usedlist, child, widget)))
    {

        calls[child->type].place(child, widget->bb.x, widget->bb.y + widget->cy, widget->bb.w, 0);
        box_expand(&widget->bb, child->bb.x, child->bb.y, child->bb.w, child->bb.h);

        widget->cy += child->bb.h;

    }

}

static void widget_window_animate(struct alfi_widget *widget, float u)
{

    struct alfi_render_window *current = &widget->animcurrent.window;
    struct alfi_render_window *target = &widget->animtarget.window;

    box_lerp(&current->background.box, target->background.box.x, target->background.box.y, target->background.box.w, target->background.box.h, u);
    color_lerp(&current->background.color, target->background.color.r, target->background.color.g, target->background.color.b, target->background.color.a, u);

}

static void widget_window_init(struct alfi_widget *widget)
{

    if (!widget->data.window.label.content)
        widget->data.window.label.content = "undefined";

}

static void widget_window_place(struct alfi_widget *widget, float x, float y, float w, float h)
{

    struct alfi_render_window *target = &widget->animtarget.window;
    struct alfi_widget *child = 0;
    float selfw = gridx * 28;
    float selfh = gridy * 28;
    float padx = gridx * 2;
    float pady = gridy * 2;
    float childw = selfw - padx * 2;
    float childh = selfh - pady * 2;

    box_move(&widget->bb, x, y);
    box_scale(&widget->bb, selfw, selfh);

    while ((child = nextchild(&usedlist, child, widget)))
    {

        calls[child->type].place(child, widget->bb.x + padx, widget->bb.y + pady, childw, childh);
        box_expand(&widget->bb, child->bb.x, child->bb.y, child->bb.w + padx, child->bb.h + pady);

    }

    switch (widget->state)
    {

    case ALFI_STATE_NORMAL:
    default:
        box_init(&target->background.box, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h);
        color_init(&target->background.color, 255, 255, 255, 255);

        break;

    }

}

static void widget_window_render(struct alfi_widget *widget, double dt)
{

    struct alfi_render_window *current = &widget->animcurrent.window;
    float feather = 16.0;
    float radius = 32;
    NVGpaint paint = nvgBoxGradient(ctx, current->background.box.x, current->background.box.y, current->background.box.w, current->background.box.h, radius, feather, nvgRGBA(0, 0, 0, 32), nvgRGBA(0, 0, 0, 0));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, current->background.box.x - marginx - feather, current->background.box.y - marginy - feather, current->background.box.w + marginx * 2 + feather * 2, current->background.box.h + marginy * 2 + feather * 2, radius);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, current->background.box.x, current->background.box.y, current->background.box.w, current->background.box.h, radius);
    nvgFillColor(ctx, nvgRGBA(current->background.color.r, current->background.color.g, current->background.color.b, current->background.color.a));
    nvgFill(ctx);

}

static unsigned int widget_window_signal(struct alfi_widget *widget, unsigned int state)
{

    return state;

}

static void checktouch(GLFWwindow *window)
{

    struct alfi_widget *root = 0;

    while ((root = nexttype(&usedlist, root, ALFI_WIDGET_WINDOW)))
    {

        struct alfi_widget *widget;

        if (!box_istouching(&root->bb, mx, my))
            continue;

        widget = findwidgetbyposition(root, mx, my);

        if (widget != hover)
        {

            if (hover)
            {

                if (calls[hover->type].signal)
                    hover->state = calls[hover->type].signal(hover, ALFI_STATE_NORMAL);

            }

            hover = widget;

            if (hover)
            {

                if (calls[hover->type].signal)
                    hover->state = calls[hover->type].signal(hover, ALFI_STATE_HOVER);

            }

        }

        if (!hover)
            continue;

        switch (hover->type)
        {

        case ALFI_WIDGET_BUTTON:
        case ALFI_WIDGET_SELECT:
            glfwSetCursor(window, cursor_hand);

            break;

        case ALFI_WIDGET_FIELD:
        case ALFI_WIDGET_HEADER:
        case ALFI_WIDGET_SUBHEADER:
        case ALFI_WIDGET_TEXT:
            glfwSetCursor(window, cursor_ibeam);

            break;

        default:
            glfwSetCursor(window, cursor_arrow);

            break;

        }

    }

}

static void reset(void)
{

    struct alfi_widget *widget = 0;

    while ((widget = nextwidget(&usedlist, widget)))
    {

        widget->cx = 0;
        widget->cy = 0;
        widget->bb.x = 0;
        widget->bb.y = 0;
        widget->bb.w = 0;
        widget->bb.h = 0;

    }

}

static void animate(unsigned int frames, float u)
{

    struct alfi_widget *widget = 0;

    while ((widget = nextwidget(&usedlist, widget)))
    {

        if (calls[widget->type].animate)
            calls[widget->type].animate(widget, u);

    }

}

static void place(void)
{

    struct alfi_widget *widget = 0;
    int offx = scrollx;
    int offy = scrolly;

    while ((widget = nexttype(&usedlist, widget, ALFI_WIDGET_WINDOW)))
    {

        calls[widget->type].place(widget, offx + gridx, offy + gridy, 0, 0);

        offx += widget->bb.w + gridx * 2;

    }

}

static void render(double dt)
{

    NVGpaint paint = nvgLinearGradient(ctx, 0, 0, 0, winh, nvgRGBA(160, 160, 160, 255), nvgRGBA(192, 192, 192, 255));
    struct alfi_widget *widget = 0;

    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, winw, winh);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);

    while ((widget = nextwidget(&usedlist, widget)))
    {

        if (calls[widget->type].render)
            calls[widget->type].render(widget, dt);

    }

}

static void init(void)
{

    struct alfi_widget *widget = 0;

    while ((widget = nextwidget(&usedlist, widget)))
    {

        if (calls[widget->type].init)
            calls[widget->type].init(widget);

        if (calls[widget->type].signal)
            calls[widget->type].signal(widget, ALFI_STATE_NORMAL);

    }

}

static void initanim(void)
{

    struct alfi_widget *widget = 0;

    while ((widget = nextwidget(&usedlist, widget)))
        widget->animcurrent = widget->animtarget;

}

static struct alfi_widget *parser_find(char *name, unsigned int group)
{

    return findwidgetbyname(&usedlist, group, name);

}

static struct alfi_widget *parser_create(unsigned int type, unsigned int group, char *in)
{

    struct list_item *item = list_pickhead(&freelist);
    struct alfi_widget *widget = item->data;

    widget->type = type;
    widget->group = group;
    widget->in.name = in;

    list_add(&usedlist, item);

    return widget;

}

static void parser_destroy(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = nextchild(&usedlist, child, widget)))
    {

        parser_destroy(child);

        child = 0;

    }

    list_move(&freelist, &usedlist, &widget->item);

}

static void parser_fail(unsigned int line)
{

    printf("Parsing failed on line %u\n", line);
    exit(EXIT_FAILURE);

}

static void load(void)
{

    unsigned int i;

    for (i = 0; i < NWIDGETS; i++)
    {

        list_inititem(&widgets[i].item, &widgets[i]);
        list_add(&freelist, &widgets[i].item);

    }

    parser.string.data = strings;
    parser.string.count = STRINGTABLESIZE;
    parser.string.offset = 0;
    parser.find = parser_find;
    parser.create = parser_create;
    parser.destroy = parser_destroy;
    parser.fail = parser_fail;

}

static void loadbase(unsigned int group)
{

    struct alfi_widget *widget;

    widget = parser_create(ALFI_WIDGET_WINDOW, group, "");

    if (widget)
    {

        widget->id.name = "window";
        widget->data.window.label.content = "undefined";

    }

    widget = parser_create(ALFI_WIDGET_VSTACK, group, "window");

    if (widget)
    {

        widget->id.name = "main";
        widget->data.vstack.halign.direction = ALFI_HALIGN_LEFT;
        widget->data.vstack.valign.direction = ALFI_VALIGN_TOP;

    }

}

static void loadfile(char *name, unsigned int group)
{

    char data[4096];
    FILE *fp;

    fp = fopen(name, "r");

    if (fp)
    {

        parser.expr.data = data;
        parser.expr.count = fread(data, 1, 4096, fp);
        parser.expr.offset = 0;
        parser.expr.line = 0;
        parser.expr.linebreak = 0;
        parser.expr.inside = 0;

        parse(&parser, group, "main");
        fclose(fp);

    }

}

static void onerror(int error, const char *desc)
{

    printf("GLFW error %d: %s\n", error, desc);

}

static void onwindowsize(GLFWwindow *window, int width, int height)
{

    winw = width;
    winh = height;

}

static void onframebuffersize(GLFWwindow *window, int width, int height)
{

    fbw = width;
    fbh = height;

    glViewport(0, 0, fbw, fbh);

}

static void onkey(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    static unsigned int active = 0;

    if (action == GLFW_PRESS)
    {

        if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
            active = 1;

        switch (key)
        {

        case GLFW_KEY_PAGE_UP:
            scrolly += winh / 2;

            break;

        case GLFW_KEY_PAGE_DOWN:
            scrolly -= winh / 2;

            break;

        case GLFW_KEY_HOME:
            scrollx += winw / 2;

            break;

        case GLFW_KEY_END:
            scrollx -= winw / 2;

            break;

        }

        if (active)
        {

            if (key == GLFW_KEY_Q)
            {

                glfwSetWindowShouldClose(window, GL_TRUE);

            }


            if (key == GLFW_KEY_R)
            {

            }

        }

    }

    if (action == GLFW_RELEASE)
    {

        if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
            active = 0;

    }

}

static void onbutton(GLFWwindow *window, int button, int action, int mods)
{

    if (action == GLFW_PRESS)
    {

        switch (button)
        {

        case GLFW_MOUSE_BUTTON_LEFT:
            if (hover && hover->id.name)
            {

                struct alfi_widget *root = findroot(&usedlist, hover->bb.x, hover->bb.y);
                struct alfi_widget *widget = 0;

                printf("?a=click&id=%s", hover->id.name);

                while ((widget = nextgroup(&usedlist, widget, root->group)))
                {

                    if (widget->id.name)
                        printf("&%s=%s", widget->id.name, "");

                }

                printf("\n");

            }

            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (hover)
            {

                parser_destroy(hover);

                hover = 0;

            }

            break;

        }

    }

}

static void oncursor(GLFWwindow *window, double x, double y)
{

    mx = x;
    my = y;

}

static void onscroll(GLFWwindow *window, double x, double y)
{

    scrollx += x * gridx;
    scrolly += y * gridy;

}

static void registercall(unsigned int type, unsigned int flags, void (*init)(struct alfi_widget *widget), void (*place)(struct alfi_widget *widget, float x, float y, float w, float h), void (*animate)(struct alfi_widget *widget, float u), void (*render)(struct alfi_widget *widget, double dt), unsigned int (*signal)(struct alfi_widget *widget, unsigned int state))
{

    calls[type].flags = flags;
    calls[type].init = init;
    calls[type].place = place;
    calls[type].animate = animate;
    calls[type].render = render;
    calls[type].signal = signal;

}

int main(int argc, char **argv)
{

    GLFWwindow *window;
    double prevt;

    if (!glfwInit())
    {

        printf("Failed to init GLFW.");

        return -1;

    }

    glfwSetErrorCallback(onerror);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(1024, 768, "Navi 0.1", 0, 0);

    if (!window)
    {

        glfwTerminate();

        return -1;

    }

    cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursor_ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursor_hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    glfwSetWindowSizeCallback(window, onwindowsize);
    glfwSetFramebufferSizeCallback(window, onframebuffersize);
    glfwSetKeyCallback(window, onkey);
    glfwSetMouseButtonCallback(window, onbutton);
    glfwSetCursorPosCallback(window, oncursor);
    glfwSetScrollCallback(window, onscroll);
    glfwMakeContextCurrent(window);

    ctx = nvgCreateGL2(0);

    if (ctx == 0)
    {

        printf("Could not init nanovg.\n");

        return -1;

    }

    fontface_regular = nvgCreateFont(ctx, "regular", "data/roboto-regular.ttf");

    if (fontface_regular == -1)
    {

        printf("Could not add font.\n");

        return -1;

    }

    fontface_bold = nvgCreateFont(ctx, "bold", "data/roboto-bold.ttf");

    if (fontface_bold == -1)
    {

        printf("Could not add font.\n");

        return -1;

    }

    fontface_icon = nvgCreateFont(ctx, "icon", "data/icofont.ttf");

    if (fontface_icon == -1)
    {

        printf("Could not add font.\n");

        return -1;

    }

    fontsize_small = gridy / 2;
    fontsize_medium = gridy / 2 + gridy / 4;
    fontsize_large = gridy;
    fontsize_xlarge = gridy + gridy;

    registercall(ALFI_WIDGET_BUTTON, ALFI_FLAG_ITEM, widget_button_init, widget_button_place, widget_button_animate, widget_button_render, widget_button_signal);
    registercall(ALFI_WIDGET_CHOICE, ALFI_FLAG_CONTAINER, 0, widget_choice_place, 0, 0, 0);
    registercall(ALFI_WIDGET_DIVIDER, ALFI_FLAG_ITEM, widget_divider_init, widget_divider_place, widget_divider_animate, widget_divider_render, widget_divider_signal);
    registercall(ALFI_WIDGET_HSTACK, ALFI_FLAG_CONTAINER, 0, widget_hstack_place, 0, 0, 0);
    registercall(ALFI_WIDGET_FIELD, ALFI_FLAG_ITEM, widget_field_init, widget_field_place, widget_field_animate, widget_field_render, widget_field_signal);
    registercall(ALFI_WIDGET_HEADER, ALFI_FLAG_ITEM, widget_header_init, widget_header_place, widget_header_animate, widget_header_render, widget_header_signal);
    registercall(ALFI_WIDGET_IMAGE, ALFI_FLAG_ITEM, widget_image_init, widget_image_place, widget_image_animate, widget_image_render, widget_image_signal);
    registercall(ALFI_WIDGET_LIST, ALFI_FLAG_CONTAINER, 0, widget_list_place, widget_list_animate, widget_list_render, widget_list_signal);
    registercall(ALFI_WIDGET_SELECT, ALFI_FLAG_ITEM, widget_select_init, widget_select_place, widget_select_animate, widget_select_render, widget_select_signal);
    registercall(ALFI_WIDGET_SUBHEADER, ALFI_FLAG_ITEM, widget_subheader_init, widget_subheader_place, widget_subheader_animate, widget_subheader_render, widget_subheader_signal);
    registercall(ALFI_WIDGET_TABLE, ALFI_FLAG_CONTAINER, 0, widget_table_place, 0, 0, 0);
    registercall(ALFI_WIDGET_TEXT, ALFI_FLAG_ITEM, widget_text_init, widget_text_place, widget_text_animate, widget_text_render, widget_text_signal);
    registercall(ALFI_WIDGET_VSTACK, ALFI_FLAG_CONTAINER, 0, widget_vstack_place, 0, 0, 0);
    registercall(ALFI_WIDGET_WINDOW, ALFI_FLAG_ITEM, widget_window_init, widget_window_place, widget_window_animate, widget_window_render, widget_window_signal);
    load();
    loadbase(1);
    loadfile("example.alfi", 1);
    loadbase(2);
    loadfile("example.alfi", 2);
    init();
    place();
    initanim();
    glfwSwapInterval(0);
    glfwSetTime(0);

    prevt = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {

        double t = glfwGetTime();
        double dt = t - prevt;
        unsigned int frames = 0;

        if (dt > 0.016)
        {

            frames++;
            prevt = t;

        }

        if (frames)
        {

            nvgBeginFrame(ctx, winw, winh, (float)fbw / (float)winw);
            reset();
            place();
            animate(frames, 0.4);
            checktouch(window);
            render(dt);
            nvgEndFrame(ctx);
            glfwSwapBuffers(window);

        }

        glfwPollEvents();

    }

    nvgDeleteGL2(ctx);
    glfwTerminate();

    return 0;

}

