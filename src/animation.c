#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "gridfmt.h"
#include "attribute.h"
#include "widget.h"
#include "animation.h"
#include "pool.h"
#include "render.h"

static struct resource *font_regular;
static struct resource *font_bold;
static struct resource *font_mono;
static struct resource *font_icon;
static struct style_color color_background;
static struct style_color color_text;
static struct style_color color_header;
static struct style_color color_focus;
static struct style_color color_line;

static float max(float a, float b)
{

    return (a > b) ? a : b;

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

    if (style_box_compare(&frame->bounds, &keyframe->bounds))
        return 1;

    for (i = 0; i < 8; i++)
    {

        if (style_compare(&frame->styles[i], &keyframe->styles[i]))
            return 1;

    }

    return 0;

}

void animation_updateframe(unsigned int type, struct frame *frame, struct frame *keyframe, float u)
{

    if (type == WIDGET_TYPE_WINDOW)
        tweenframe(frame, keyframe, 1.0);
    else
        tweenframe(frame, keyframe, u);

    frame->animating = compareframe(frame, keyframe);

}

void animation_initframe(struct frame *frame, int x, int y, int w, int h)
{

    unsigned int i;

    style_box_init(&frame->bounds, x, y, w, h, 0);

    for (i = 0; i < 8; i++)
        style_init(&frame->styles[i]);

}

static void anchor_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_anchor *payload = &widget->payload.anchor;
    struct style *label = &frame->styles[0];

    style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_focus);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->marginw, view->marginh);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    frame->bounds.h = label->box.h + view->marginh * 2;

}

static void anchor_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_anchor *payload = &widget->payload.anchor;
    struct style *label = &frame->styles[0];

    render_filltext(label, payload->label.content);

}

static unsigned int anchor_getcursor(struct frame *frame, int x, int y)
{

    struct style *label = &frame->styles[0];

    if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void button_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_button *payload = &widget->payload.button;
    struct style *surface = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *icon = &frame->styles[2];

    /* Label */
    style_font_init(&label->font, font_bold->index, view->fontsizemedium, STYLE_ALIGN_CENTER | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_focus);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->unitw, view->unith);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_color_clone(&label->color, &color_background);

    /* Icon */
    style_font_init(&icon->font, font_icon->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&icon->color, &color_focus);
    style_box_init(&icon->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&icon->box, view->unitw, view->unith);
    style_box_scale(&icon->box, view->fontsizemedium, view->fontsizemedium);
    style_box_translate(&icon->box, 0, 24);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_color_clone(&icon->color, &color_background);

    /* Surface */
    style_box_init(&surface->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 32);
    style_color_clone(&surface->color, &color_line);
    style_box_shrink(&surface->box, view->marginw, view->marginh);
    style_box_expand(&surface->box, &label->box, view->unitw - view->marginw, view->unith - view->marginh);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_color_clone(&surface->color, &color_focus);

    frame->bounds.h = surface->box.h + 2 * view->marginh;

}

static void button_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_button *payload = &widget->payload.button;
    struct style *surface = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *icon = &frame->styles[2];

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        render_fillrect(surface);
    else
        render_fillrectborder(surface, 2.0);

    render_filltext(label, payload->label.content);

    if (payload->icon.type)
        render_fillicon(icon, payload->icon.type);

}

static unsigned int button_getcursor(struct frame *frame, int x, int y)
{

    struct style *surface = &frame->styles[0];

    if (style_box_istouching(&surface->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void choice_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_choice *payload = &widget->payload.choice;
    struct style *background = &frame->styles[0];
    struct style *label = &frame->styles[1];

    /* Label */
    style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_text);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->marginw, view->marginh);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    if (widget->header.state == WIDGET_STATE_HOVER)
        style_color_clone(&background->color, &color_line);

    /* Background */
    style_box_init(&background->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 4);
    style_box_expand(&background->box, &label->box, view->marginw, view->marginh);

    frame->bounds.h = background->box.h;

}

static void choice_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_choice *payload = &widget->payload.choice;
    struct style *background = &frame->styles[0];
    struct style *label = &frame->styles[1];

    if (widget->header.state == WIDGET_STATE_HOVER)
        render_fillrectborder(background, 2.0);

    render_filltext(label, payload->label.content);

}

static unsigned int choice_getcursor(struct frame *frame, int x, int y)
{

    return ANIMATION_CURSOR_HAND;

}

static void code_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_code *payload = &widget->payload.code;
    struct style *label = &frame->styles[0];

    style_font_init(&label->font, font_mono->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_text);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->marginw, view->marginh);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    frame->bounds.h = label->box.h + view->marginh * 2;

}

static void code_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_code *payload = &widget->payload.code;
    struct style *label = &frame->styles[0];

    render_filltext(label, payload->label.content);

}

static unsigned int code_getcursor(struct frame *frame, int x, int y)
{

    struct style *label = &frame->styles[0];

    if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void divider_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_divider *payload = &widget->payload.divider;
    struct style *line = &frame->styles[0];
    struct style *label = &frame->styles[1];

    /* Label */
    style_font_init(&label->font, font_regular->index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_text);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);

    if (strlen(payload->label.content))
    {

        style_box_scale(&label->box, render_textwidth(label, payload->label.content), render_textheight(label, payload->label.content));
        style_box_translate(&label->box, frame->bounds.w / 2 - label->box.w / 2, view->marginh - view->fontsizesmall / 2);

    }

    /* Line */
    style_color_clone(&line->color, &color_line);
    style_box_init(&line->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&line->box, view->marginw, view->marginh);
    style_box_scale(&line->box, line->box.w, 2);

    frame->bounds.h = line->box.h + view->marginh * 2;

}

static void divider_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_divider *payload = &widget->payload.divider;
    struct style *line = &frame->styles[0];
    struct style *label = &frame->styles[1];

    if (strlen(payload->label.content))
    {

        render_fillrectgap(line, label->box.x - view->marginw, label->box.w + view->marginw * 2);
        render_filltext(label, payload->label.content);

    }

    else
    {

        render_fillrect(line);

    }

}

static unsigned int divider_getcursor(struct frame *frame, int x, int y)
{

    struct style *label = &frame->styles[1];

    if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void field_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_field *payload = &widget->payload.field;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];

    /* Data */
    style_font_init(&data->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&data->color, &color_text);
    style_box_init(&data->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&data->box, view->unitw, view->unith);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_box_shrink(&data->box, view->marginw, view->marginh);

    style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));

    /* Label */
    style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_line);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
        style_font_init(&label->font, font_regular->index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&label->color, &color_focus);

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
        style_box_shrink(&label->box, view->unitw, view->marginh - label->font.size / 2);
    else
        style_box_shrink(&label->box, view->unitw, view->unith);

    style_box_scale(&label->box, render_textwidth(label, payload->label.content), render_textheight(label, payload->label.content));

    /* Border */
    style_color_clone(&border->color, &color_line);
    style_box_init(&border->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 4);
    style_box_shrink(&border->box, view->marginw, view->marginh);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&border->color, &color_focus);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_box_expand(&border->box, &data->box, view->unitw, view->unith);
    else
        style_box_expand(&border->box, &data->box, view->unitw - view->marginw, view->unith - view->marginh);

    frame->bounds.h = border->box.h + view->marginh * 2;

}

static void field_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_field *payload = &widget->payload.field;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
    {

        if (strlen(payload->label.content))
        {

            render_fillrectbordergap(border, 2.0, label->box.x - view->marginw, label->box.w + view->marginw * 2);
            render_filltext(label, payload->label.content);

        }

        else
        {

            render_fillrectborder(border, 2.0);

        }

    }

    else
    {

        render_fillrectborder(border, 2.0);
        render_filltext(label, payload->label.content);

    }

    if (widget->header.state == WIDGET_STATE_FOCUS)
        render_filltextinput(data, payload->data.content, payload->data.offset, &border->color);
    else
        render_filltext(data, payload->data.content);

}

static unsigned int field_getcursor(struct frame *frame, int x, int y)
{

    struct style *border = &frame->styles[0];

    if (style_box_istouching(&border->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void header_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_header *payload = &widget->payload.header;
    struct style *label = &frame->styles[0];

    style_font_init(&label->font, font_bold->index, view->fontsizexlarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_header);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->marginw, view->marginh);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    frame->bounds.h = label->box.h + view->marginh * 2;

}

static void header_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_header *payload = &widget->payload.header;
    struct style *label = &frame->styles[0];

    render_filltext(label, payload->label.content);

}

static unsigned int header_getcursor(struct frame *frame, int x, int y)
{

    struct style *label = &frame->styles[0];

    if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void header2_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_header2 *payload = &widget->payload.header2;
    struct style *label = &frame->styles[0];

    style_font_init(&label->font, font_bold->index, view->fontsizelarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_header);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->marginw, view->marginh);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    frame->bounds.h = label->box.h + view->marginh * 2;

}

static void header2_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_header2 *payload = &widget->payload.header2;
    struct style *label = &frame->styles[0];

    render_filltext(label, payload->label.content);

}

static unsigned int header2_getcursor(struct frame *frame, int x, int y)
{

    struct style *label = &frame->styles[0];

    if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void header3_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_header3 *payload = &widget->payload.header3;
    struct style *label = &frame->styles[0];

    style_font_init(&label->font, font_bold->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_header);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->marginw, view->marginh);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    frame->bounds.h = label->box.h + view->marginh * 2;

}

static void header3_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_header3 *payload = &widget->payload.header3;
    struct style *label = &frame->styles[0];

    render_filltext(label, payload->label.content);

}

static unsigned int header3_getcursor(struct frame *frame, int x, int y)
{

    struct style *label = &frame->styles[0];

    if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void image_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_image *payload = &widget->payload.image;
    struct resource *resource = pool_resource_find(payload->link.url);
    struct style *surface = &frame->styles[0];
    float ratio;

    if (!resource)
        return;

    ratio = (float)frame->bounds.w / (float)resource->w;

    style_box_init(&surface->box, frame->bounds.x, frame->bounds.y, resource->w * ratio, resource->h * ratio, 0);
    style_box_shrink(&surface->box, view->marginw, view->marginh);

    /* Fixed size */
    /*
    style_box_init(&surface->box, frame->bounds.x, frame->bounds.y, resource->w, resource->h, 0);
    style_box_translate(&surface->box, view->marginw, view->marginh);
    */

    frame->bounds.h = surface->box.h + view->marginh * 2;

}

static void image_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_image *payload = &widget->payload.image;
    struct resource *resource = pool_resource_find(payload->link.url);
    struct style *surface = &frame->styles[0];

    if (!resource)
        return;

    render_fillimage(surface, resource->index);

}

static void list_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct style *dot = &frame->styles[0];
    struct widget *child = 0;
    int pw = view->unitw * 1;
    int cx = frame->bounds.x + pw;
    int cy = frame->bounds.y;
    int cw = frame->bounds.w - pw * 2;
    int ch = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        struct frame *cframe = pool_getframe(child->index);
        struct frame keyframe;

        animation_initframe(&keyframe, cx, cy, cw, 0);
        animation_step(child, &keyframe, view, u);
        animation_updateframe(child->header.type, cframe, &keyframe, u);

        ch = max(ch, cy + cframe->bounds.h - frame->bounds.y);
        cy += cframe->bounds.h;

    }

    style_color_clone(&dot->color, &color_header);

    frame->bounds.h = ch;

}

static void list_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct style *dot = &frame->styles[0];
    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        animation_render(child, view);

        if (child->header.type != WIDGET_TYPE_LIST)
        {

            struct frame *cframe = pool_getframe(child->index);

            style_box_init(&dot->box, cframe->bounds.x - view->unitw / 2 + view->marginw, cframe->bounds.y + cframe->bounds.h / 2, 0, 0, 3);
            render_fillcircle(dot);

        }

    }

}

static void select_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_select *payload = &widget->payload.select;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];
    struct widget *child = 0;
    int cx = frame->bounds.x + view->unitw;
    int cy = frame->bounds.y + view->unith * 3;
    int cw = frame->bounds.w - view->unitw * 2;
    int ch = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        struct frame *cframe = pool_getframe(child->index);
        struct frame keyframe;

        animation_initframe(&keyframe, cx, cy, cw, 0);
        animation_step(child, &keyframe, view, u);
        animation_updateframe(child->header.type, cframe, &keyframe, u);

        if (widget->header.state == WIDGET_STATE_FOCUS)
        {

            ch = max(ch, cy + cframe->bounds.h - frame->bounds.y);
            cy += cframe->bounds.h;

        }

    }

    /* Data */
    style_font_init(&data->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&data->color, &color_text);
    style_box_init(&data->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&data->box, view->unitw, view->unith);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_box_shrink(&data->box, view->marginw, view->marginh);

    style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));

    /* Label */
    style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_line);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
        style_font_init(&label->font, font_regular->index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&label->color, &color_focus);

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
        style_box_shrink(&label->box, view->unitw, view->marginh - label->font.size / 2);
    else
        style_box_shrink(&label->box, view->unitw, view->unith);

    style_box_scale(&label->box, render_textwidth(label, payload->label.content), render_textheight(label, payload->label.content));

    /* Border */
    style_color_clone(&border->color, &color_line);
    style_box_init(&border->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 4);
    style_box_shrink(&border->box, view->marginw, view->marginh);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&border->color, &color_focus);

    if (widget->header.state == WIDGET_STATE_FOCUS)
    {

        style_box_expand(&border->box, &data->box, view->unitw, view->unith);

        border->box.h += ch - view->unith * 2 - view->marginh;

    }

    else
    {

        style_box_expand(&border->box, &data->box, view->unitw - view->marginw, view->unith - view->marginh);

    }

    frame->bounds.h = border->box.h + view->marginh * 2;

}

static void select_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_select *payload = &widget->payload.select;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];
    struct widget *child = 0;

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
    {

        if (strlen(payload->label.content))
        {

            render_fillrectbordergap(border, 2.0, label->box.x - view->marginw, label->box.w + view->marginw * 2);
            render_filltext(label, payload->label.content);

        }

        else
        {

            render_fillrectborder(border, 2.0);

        }

    }

    else
    {

        render_fillrectborder(border, 2.0);
        render_filltext(label, payload->label.content);

    }

    render_filltext(data, payload->data.content);

    if (widget->header.state == WIDGET_STATE_FOCUS)
    {

        while ((child = pool_widget_nextchild(child, widget)))
            animation_render(child, view);

    }

}

static unsigned int select_getcursor(struct frame *frame, int x, int y)
{

    struct style *border = &frame->styles[0];

    if (style_box_istouching(&border->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void table_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_table *payload = &widget->payload.table;
    unsigned int gsize;
    struct widget *child = 0;
    unsigned int i;
    int cx = frame->bounds.x;
    int cy = frame->bounds.y;
    int ch = 0;

    if (strlen(payload->grid.format))
        gsize = gridfmt_size(payload->grid.format);

    for (i = 0; (child = pool_widget_nextchild(child, widget)); i++)
    {

        int cw = frame->bounds.w;

        if (strlen(payload->grid.format))
            cw = gridfmt_colsize(payload->grid.format, i % gsize) * view->unitw * 2;

        if (cx + cw <= frame->bounds.x + frame->bounds.w)
        {

            struct frame *cframe = pool_getframe(child->index);
            struct frame keyframe;

            animation_initframe(&keyframe, cx, cy, cw, 0);
            animation_step(child, &keyframe, view, u);
            animation_updateframe(child->header.type, cframe, &keyframe, u);

            ch = max(ch, cy + cframe->bounds.h - frame->bounds.y);

        }

        cx += cw;

        if (cx >= frame->bounds.x + frame->bounds.w)
        {

            cx = frame->bounds.x;
            cy = frame->bounds.y + ch;

        }

    }

    frame->bounds.h = ch;

}

static void table_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
        animation_render(child, view);

}

static void text_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_text *payload = &widget->payload.text;
    struct style *label = &frame->styles[0];

    style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_text);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, view->marginw, view->marginh);
    style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    frame->bounds.h = label->box.h + view->marginh * 2;

}

static void text_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_text *payload = &widget->payload.text;
    struct style *label = &frame->styles[0];

    render_filltext(label, payload->label.content);

}

static unsigned int text_getcursor(struct frame *frame, int x, int y)
{

    struct style *label = &frame->styles[0];

    if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void toggle_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget_payload_toggle *payload = &widget->payload.toggle;
    struct style *groove = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *ohandle = &frame->styles[2];
    struct style *ihandle = &frame->styles[3];

    /* IHandle */
    style_color_clone(&ihandle->color, &color_background);
    style_box_init(&ihandle->box, frame->bounds.x, frame->bounds.y, view->unitw * 2 - view->unitw / 2, view->fontsizemedium, 14);
    style_box_translate(&ihandle->box, view->marginw * 2, view->marginh * 2 + 4);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_box_translate(&ihandle->box, 2 * view->unitw - 2 * view->marginw, 0);

    /* OHandle */
    style_color_clone(&ohandle->color, &color_text);
    style_box_init(&ohandle->box, frame->bounds.x, frame->bounds.y, view->unitw * 2 - view->unitw / 2, view->fontsizemedium, 16);
    style_box_translate(&ohandle->box, view->marginw * 2, view->marginh * 2 + 4);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&ohandle->color, &color_focus);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_box_translate(&ohandle->box, 2 * view->unitw - 2 * view->marginw, 0);

    /* Label */
    style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&label->color, &color_text);
    style_box_init(&label->box, frame->bounds.x, frame->bounds.y, frame->bounds.w, frame->bounds.h, 0);
    style_box_shrink(&label->box, 3 * view->unitw + view->marginw, view->marginh);
    style_box_scale(&label->box, render_textwidth(label, payload->label.content), render_textheight(label, payload->label.content));

    /* Groove */
    style_color_clone(&groove->color, &color_line);
    style_box_init(&groove->box, frame->bounds.x, frame->bounds.y, 2 * view->unitw, view->fontsizemedium, 8);
    style_box_translate(&groove->box, view->marginw, view->marginh);
    style_box_shrink(&groove->box, 4, 4);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_color_clone(&groove->color, &color_focus);

    frame->bounds.h = label->box.h + view->marginh * 2;

}

static void toggle_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_toggle *payload = &widget->payload.toggle;
    struct style *groove = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *ohandle = &frame->styles[2];
    struct style *ihandle = &frame->styles[3];

    render_fillrect(groove);
    render_fillcircle(ohandle);
    render_fillcircle(ihandle);
    render_filltext(label, payload->label.content);

}

static unsigned int toggle_getcursor(struct frame *frame, int x, int y)
{

    struct style *groove = &frame->styles[0];
    struct style *label = &frame->styles[1];

    if (style_box_istouching(&groove->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else if (style_box_istouching(&label->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static void window_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    struct widget *child = 0;
    int cx = frame->bounds.x;
    int cy = frame->bounds.y;
    int cw = frame->bounds.w;
    int ch = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        struct frame *cframe = pool_getframe(child->index);
        struct frame keyframe;

        animation_initframe(&keyframe, cx, cy, cw, 0);
        animation_step(child, &keyframe, view, u);
        animation_updateframe(child->header.type, cframe, &keyframe, u);

        ch = max(ch, cy + cframe->bounds.h - frame->bounds.y);

    }

    style_color_clone(&frame->styles[0].color, &color_background);

    frame->bounds.h = ch;

}

static void window_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
        animation_render(child, view);

}

void animation_step(struct widget *widget, struct frame *frame, struct view *view, float u)
{

    switch (widget->header.type)
    {

    case WIDGET_TYPE_ANCHOR:
        anchor_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_BUTTON:
        button_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_CHOICE:
        choice_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_CODE:
        code_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_DIVIDER:
        divider_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_FIELD:
        field_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_HEADER:
        header_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_HEADER2:
        header2_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_HEADER3:
        header3_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_IMAGE:
        image_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_LIST:
        list_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_SELECT:
        select_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_TABLE:
        table_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_TEXT:
        text_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_TOGGLE:
        toggle_step(widget, frame, view, u);

        break;

    case WIDGET_TYPE_WINDOW:
        window_step(widget, frame, view, u);

        break;

    }

}

void animation_render(struct widget *widget, struct view *view)
{

    struct frame *frame = pool_getframe(widget->index);

    switch (widget->header.type)
    {

    case WIDGET_TYPE_ANCHOR:
        anchor_render(widget, frame, view);

        break;

    case WIDGET_TYPE_BUTTON:
        button_render(widget, frame, view);

        break;

    case WIDGET_TYPE_CHOICE:
        choice_render(widget, frame, view);

        break;

    case WIDGET_TYPE_CODE:
        code_render(widget, frame, view);

        break;

    case WIDGET_TYPE_DIVIDER:
        divider_render(widget, frame, view);

        break;

    case WIDGET_TYPE_FIELD:
        field_render(widget, frame, view);

        break;

    case WIDGET_TYPE_HEADER:
        header_render(widget, frame, view);

        break;

    case WIDGET_TYPE_HEADER2:
        header2_render(widget, frame, view);

        break;

    case WIDGET_TYPE_HEADER3:
        header3_render(widget, frame, view);

        break;

    case WIDGET_TYPE_IMAGE:
        image_render(widget, frame, view);

        break;

    case WIDGET_TYPE_LIST:
        list_render(widget, frame, view);

        break;

    case WIDGET_TYPE_SELECT:
        select_render(widget, frame, view);

        break;

    case WIDGET_TYPE_TABLE:
        table_render(widget, frame, view);

        break;

    case WIDGET_TYPE_TEXT:
        text_render(widget, frame, view);

        break;

    case WIDGET_TYPE_TOGGLE:
        toggle_render(widget, frame, view);

        break;

    case WIDGET_TYPE_WINDOW:
        window_render(widget, frame, view);

        break;

    }

}

unsigned int animation_getcursor(struct widget *widget, int x, int y)
{

    struct frame *frame = pool_getframe(widget->index);

    switch (widget->header.type)
    {

    case WIDGET_TYPE_ANCHOR:
        return anchor_getcursor(frame, x, y);

    case WIDGET_TYPE_BUTTON:
        return button_getcursor(frame, x, y);

    case WIDGET_TYPE_CHOICE:
        return choice_getcursor(frame, x, y);

    case WIDGET_TYPE_CODE:
        return code_getcursor(frame, x, y);

    case WIDGET_TYPE_DIVIDER:
        return divider_getcursor(frame, x, y);

    case WIDGET_TYPE_FIELD:
        return field_getcursor(frame, x, y);

    case WIDGET_TYPE_HEADER:
        return header_getcursor(frame, x, y);

    case WIDGET_TYPE_HEADER2:
        return header2_getcursor(frame, x, y);

    case WIDGET_TYPE_HEADER3:
        return header3_getcursor(frame, x, y);

    case WIDGET_TYPE_SELECT:
        return select_getcursor(frame, x, y);

    case WIDGET_TYPE_TEXT:
        return text_getcursor(frame, x, y);

    case WIDGET_TYPE_TOGGLE:
        return toggle_getcursor(frame, x, y);

    }

    return ANIMATION_CURSOR_ARROW;

}

void animation_setupfonts(void)
{

    font_regular = render_loadfont("file:///usr/share/navi/roboto-regular.ttf");
    font_bold = render_loadfont("file:///usr/share/navi/roboto-bold.ttf");
    font_mono = render_loadfont("file:///usr/share/navi/robotomono-regular.ttf");
    font_icon = render_loadfont("file:///usr/share/navi/icofont.ttf");

}

void animation_settheme(unsigned int type)
{

    switch (type)
    {

    case ANIMATION_THEME_LIGHT:
        style_color_init(&color_background, 255, 255, 255, 255);
        style_color_init(&color_text, 96, 96, 96, 255);
        style_color_init(&color_header, 64, 64, 64, 255);
        style_color_init(&color_focus, 96, 192, 192, 255);
        style_color_init(&color_line, 192, 192, 192, 255);

        break;

    case ANIMATION_THEME_DARK:
        style_color_init(&color_background, 24, 24, 24, 255);
        style_color_init(&color_text, 192, 192, 192, 255);
        style_color_init(&color_header, 224, 224, 224, 255);
        style_color_init(&color_focus, 96, 192, 192, 255);
        style_color_init(&color_line, 64, 64, 64, 255);

        break;

    }

}

