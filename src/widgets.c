#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "widgets.h"
#include "view.h"
#include "call.h"
#include "pool.h"
#include "render.h"

static struct resource_font res_font_regular;
static struct resource_font res_font_bold;
static struct resource_font res_font_mono;
static struct resource_font res_font_icon;
static struct style_color color_background;
static struct style_color color_text;
static struct style_color color_header;
static struct style_color color_focus;
static struct style_color color_focustext;
static struct style_color color_line;

static void anchor_create(struct widget *widget)
{

    struct payload_anchor *payload = &widget->payload.anchor;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

static void anchor_destroy(struct widget *widget)
{

    struct payload_anchor *payload = &widget->payload.anchor;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

static int anchor_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_anchor *payload = &widget->payload.anchor;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_focus);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void anchor_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_anchor *payload = &widget->payload.anchor;
    struct style *text = &frame->styles[0];

    render_filltext(text, payload->label.content);

}

static unsigned int anchor_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int anchor_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ALFI_CURSOR_HAND;
    else
        return ALFI_CURSOR_ARROW;

}

static void button_create(struct widget *widget)
{

    struct payload_button *payload = &widget->payload.button;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");
    payload->link.url = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.url, "");
    payload->link.mime = pool_string_create(ALFI_ATTRIBUTE_LINK, payload->link.mime, "");

}

static void button_destroy(struct widget *widget)
{

    struct payload_button *payload = &widget->payload.button;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->link.url = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.url);
    payload->link.mime = pool_string_destroy(ALFI_ATTRIBUTE_LINK, payload->link.mime);

}

static int button_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_button *payload = &widget->payload.button;
    struct style *surface = &frame->styles[0];
    struct style *text = &frame->styles[1];

    style_font_init(&text->font, res_font_bold.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (payload->mode.mode == ALFI_MODE_ON)
        style_color_clone(&text->color, &color_focustext);
    else
        style_color_clone(&text->color, &color_text);

    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, 3 * view->marginw, 3 * view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    if (payload->mode.mode == ALFI_MODE_ON)
        style_color_clone(&surface->color, &color_focus);
    else
        style_color_clone(&surface->color, &color_line);

    style_box_init(&surface->box, x, y, w, 0, 4);
    style_box_shrink(&surface->box, view->marginw, view->marginh);
    style_box_expand(&surface->box, &text->box, 0, 2 * view->marginh);

    return surface->box.h + 2 * view->marginh;

}

static void button_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_button *payload = &widget->payload.button;
    struct style *surface = &frame->styles[0];
    struct style *text = &frame->styles[1];

    render_fillrect(surface);

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int button_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static unsigned int button_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *surface = &frame->styles[0];

    if (style_box_istouching(&surface->box, x, y))
        return ALFI_CURSOR_HAND;
    else
        return ALFI_CURSOR_ARROW;

}

static void choice_create(struct widget *widget)
{

    struct payload_choice *payload = &widget->payload.choice;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void choice_destroy(struct widget *widget)
{

    struct payload_choice *payload = &widget->payload.choice;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static int choice_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_choice *payload = &widget->payload.choice;
    struct style *background = &frame->styles[0];
    struct style *text = &frame->styles[1];

    style_font_init(&text->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    if (widget->header.state == ALFI_STATE_HOVER)
        style_color_clone(&background->color, &color_line);

    style_box_init(&background->box, x, y, w, 0, 4);
    style_box_expand(&background->box, &text->box, view->marginw, view->marginh);

    return background->box.h;

}

static void choice_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_choice *payload = &widget->payload.choice;
    struct style *background = &frame->styles[0];
    struct style *text = &frame->styles[1];

    if (widget->header.state == ALFI_STATE_HOVER)
        render_fillrect(background);

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int choice_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int choice_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_HAND;

}

static void divider_create(struct widget *widget)
{

    struct payload_divider *payload = &widget->payload.divider;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void divider_destroy(struct widget *widget)
{

    struct payload_divider *payload = &widget->payload.divider;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static int divider_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_divider *payload = &widget->payload.divider;
    struct style *line = &frame->styles[0];
    struct style *text = &frame->styles[1];

    style_font_init(&text->font, res_font_regular.index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);

    if (strlen(payload->label.content))
    {

        style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));
        style_box_translate(&text->box, w / 2 - text->box.w / 2, view->unith / 2 + 2);

    }

    style_color_clone(&line->color, &color_line);
    style_box_init(&line->box, x + view->marginw, y + view->unith - 1, w - view->marginw * 2, 2, 0);

    return view->unith * 2;

}

static void divider_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_divider *payload = &widget->payload.divider;
    struct style *line = &frame->styles[0];
    struct style *text = &frame->styles[1];

    if (strlen(payload->label.content))
    {

        render_fillrectgap(line, text->box.x - view->marginw, text->box.w + view->marginw * 2);
        render_filltext(text, payload->label.content);

    }

    else
    {

        render_fillrect(line);

    }

}

static unsigned int divider_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int divider_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[1];

    if (style_box_istouching(&text->box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void field_create(struct widget *widget)
{

    struct payload_field *payload = &widget->payload.field;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void field_destroy(struct widget *widget)
{

    struct payload_field *payload = &widget->payload.field;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

static int field_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_field *payload = &widget->payload.field;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];

    style_font_init(&data->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&data->color, &color_text);
    style_box_init(&data->box, x, y, w, 0, 0);

    if (widget->header.state == ALFI_STATE_FOCUS)
        style_box_shrink(&data->box, view->unitw + view->marginw, view->unith + view->marginh);
    else
        style_box_shrink(&data->box, view->unitw, view->unith);

    style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));
    style_box_init(&label->box, x, y, w, 0, 0);

    if (widget->header.state == ALFI_STATE_FOCUS || strlen(payload->data.content))
        style_font_init(&label->font, res_font_regular.index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    else
        style_font_init(&label->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (widget->header.state == ALFI_STATE_FOCUS)
        style_color_clone(&label->color, &color_focus);
    else
        style_color_clone(&label->color, &color_line);

    if (widget->header.state == ALFI_STATE_FOCUS || strlen(payload->data.content))
    {

        style_box_shrink(&label->box, view->unitw, view->marginh - label->font.size / 2);
        style_box_scale(&label->box, render_textwidth(label, payload->label.content), render_textheight(label, payload->label.content));

    }

    else
    {

        style_box_shrink(&label->box, view->unitw, view->unith);
        style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    }

    style_box_init(&border->box, x, y, w, 0, 4);
    style_box_shrink(&border->box, view->marginw, view->marginh);

    if (widget->header.state == ALFI_STATE_FOCUS)
        style_color_clone(&border->color, &color_focus);
    else
        style_color_clone(&border->color, &color_line);

    if (widget->header.state == ALFI_STATE_FOCUS)
        style_box_expand(&border->box, &data->box, view->unitw, view->unith);
    else
        style_box_expand(&border->box, &data->box, view->unitw - view->marginw, view->unith - view->marginh);

    return border->box.h + view->marginh * 2;

}

static void field_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_field *payload = &widget->payload.field;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];

    if (widget->header.state == ALFI_STATE_FOCUS || strlen(payload->data.content))
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

        if (strlen(payload->label.content))
            render_filltext(label, payload->label.content);

    }

    if (strlen(payload->data.content))
    {

        if (widget->header.state == ALFI_STATE_FOCUS)
            render_filltextinput(data, payload->data.content, payload->data.offset, &border->color);
        else
            render_filltext(data, payload->data.content);

    }

}

static unsigned int field_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static unsigned int field_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *data = &frame->styles[2];

    if (style_box_istouching(&data->box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void header_create(struct widget *widget)
{

    struct payload_header *payload = &widget->payload.header;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void header_destroy(struct widget *widget)
{

    struct payload_header *payload = &widget->payload.header;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static int header_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_header *payload = &widget->payload.header;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, res_font_bold.index, view->fontsizexlarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_header);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void header_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_header *payload = &widget->payload.header;
    struct style *text = &frame->styles[0];

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int header_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int header_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void image_create(struct widget *widget)
{

}

static void image_destroy(struct widget *widget)
{

    struct resource_image *resource = &widget->resource.image;

    render_unloadimage(resource);

}

static int image_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct resource_image *resource = &widget->resource.image;
    struct style *surface = &frame->styles[0];

    /* This is if you want to scale with an aspect ration
    float ratio = (float)w / (float)resource->w;
    style_box_init(&surface->box, x, y, resource->w * ratio, resource->h * ratio, 0);
    style_box_shrink(&surface->box, view->marginw, view->marginh);
    */

    style_box_init(&surface->box, x, y, resource->w, resource->h, 0);
    style_box_translate(&surface->box, view->marginw, view->marginh);

    return surface->box.h + view->marginh * 2;

}

static void image_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct resource_image *resource = &widget->resource.image;
    struct style *surface = &frame->styles[0];

    render_fillimage(surface, resource->ref);

}

static unsigned int image_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int image_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

static void list_create(struct widget *widget)
{

}

static void list_destroy(struct widget *widget)
{

}

static int list_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget *child = 0;
    int pw = view->unitw * 1;
    int cx = x + pw;
    int cy = y;
    int cw = w - pw * 2;
    int h = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = call_animate(child, cx, cy, cw, view, u);

        if (h < cy + ch - y)
            h = cy + ch - y;

        cy += ch;

    }

    return h;

}

static void list_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        call_render(child, view);

        if (child->header.type != ALFI_WIDGET_LIST)
        {

            struct style dot;

            style_init(&dot);
            style_color_clone(&dot.color, &color_header);
            style_box_init(&dot.box, child->frame.bounds.x - view->unitw / 2 + view->marginw, child->frame.bounds.y + child->frame.bounds.h / 2, 0, 0, 3);
            render_fillcircle(&dot);

        }

    }

}

static unsigned int list_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int list_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

static void select_create(struct widget *widget)
{

    struct payload_select *payload = &widget->payload.select;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");
    payload->data.content = pool_allocate(ALFI_ATTRIBUTE_DATA, payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void select_destroy(struct widget *widget)
{

    struct payload_select *payload = &widget->payload.select;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);
    payload->data.content = pool_string_destroy(ALFI_ATTRIBUTE_DATA, payload->data.content);

}

static int select_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_select *payload = &widget->payload.select;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];
    struct widget *child = 0;
    int cx = view->unitw;
    int cy = view->unith * 3;
    int cw = w - view->unitw * 2;
    int h = view->unith * 3;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = call_animate(child, x + cx, y + cy, cw, view, u);

        cy += ch;

        if (widget->header.state == ALFI_STATE_FOCUS)
        {

            if (h < cy + view->unith)
                h = cy + view->unith;

        }

    }

    style_font_init(&data->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&data->color, &color_text);
    style_box_init(&data->box, x, y, w, h, 0);

    if (widget->header.state == ALFI_STATE_FOCUS)
    {

        style_box_shrink(&data->box, view->unitw + view->marginw, view->unith + view->marginh);
        style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));

    }

    else
    {

        style_box_shrink(&data->box, view->unitw, view->unith);
        style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));

    }

    if (widget->header.state == ALFI_STATE_FOCUS || strlen(payload->data.content))
        style_font_init(&label->font, res_font_regular.index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    else
        style_font_init(&label->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (widget->header.state == ALFI_STATE_FOCUS)
        style_color_clone(&label->color, &color_focus);
    else
        style_color_clone(&label->color, &color_line);

    style_box_init(&label->box, x, y, w, h, 0);

    if (widget->header.state == ALFI_STATE_FOCUS || strlen(payload->data.content))
    {

        style_box_translate(&label->box, view->unitw, 0);
        style_box_scale(&label->box, render_textwidth(label, payload->label.content), render_textheight(label, payload->label.content));

    }

    else
    {

        style_box_translate(&label->box, view->unitw, view->unith);
        style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    }

    if (widget->header.state == ALFI_STATE_FOCUS)
        style_color_clone(&border->color, &color_focus);
    else
        style_color_clone(&border->color, &color_line);

    style_box_init(&border->box, x, y, w, h, 4);
    style_box_shrink(&border->box, view->marginw, view->marginh);

    return border->box.h + view->marginh * 2;

}

static void select_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_select *payload = &widget->payload.select;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];
    struct widget *child = 0;

    if (widget->header.state == ALFI_STATE_FOCUS || strlen(payload->data.content))
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

        if (strlen(payload->label.content))
            render_filltext(label, payload->label.content);

    }

    if (strlen(payload->data.content))
        render_filltext(data, payload->data.content);

    if (widget->header.state == ALFI_STATE_FOCUS)
    {

        while ((child = pool_widget_nextchild(child, widget)))
            call_render(child, view);

    }

}

static unsigned int select_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static unsigned int select_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *border = &frame->styles[0];

    if (style_box_istouching(&border->box, x, y))
        return ALFI_CURSOR_HAND;
    else
        return ALFI_CURSOR_ARROW;

}

static void subheader_create(struct widget *widget)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void subheader_destroy(struct widget *widget)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static int subheader_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_subheader *payload = &widget->payload.subheader;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, res_font_bold.index, view->fontsizelarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_header);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void subheader_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_subheader *payload = &widget->payload.subheader;
    struct style *text = &frame->styles[0];

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int subheader_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int subheader_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void table_create(struct widget *widget)
{

}

static void table_destroy(struct widget *widget)
{

}

static int table_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_table *payload = &widget->payload.table;
    struct widget *child = 0;
    int cx = x;
    int cy = y;
    int cw = view->unitw * payload->grid.csize * 2;
    int h = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = call_animate(child, cx, cy, cw, view, u);

        if (h < cy + ch - y)
            h = cy + ch - y;

        cx += cw;

        if (cx >= x + w)
        {

            cx = x;
            cy = y + h;

        }

    }

    return h;

}

static void table_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
        call_render(child, view);

}

static unsigned int table_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int table_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

static void text_create(struct widget *widget)
{

    struct payload_text *payload = &widget->payload.text;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void text_destroy(struct widget *widget)
{

    struct payload_text *payload = &widget->payload.text;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static int text_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_text *payload = &widget->payload.text;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void text_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_text *payload = &widget->payload.text;
    struct style *text = &frame->styles[0];

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int text_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int text_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void toggle_create(struct widget *widget)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "");

}

static void toggle_destroy(struct widget *widget)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static int toggle_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_toggle *payload = &widget->payload.toggle;
    struct style *groove = &frame->styles[0];
    struct style *text = &frame->styles[1];
    struct style *ohandle = &frame->styles[2];
    struct style *ihandle = &frame->styles[3];

    style_color_clone(&ihandle->color, &color_background);
    style_box_init(&ihandle->box, x, y, view->unitw * 2 - view->unitw / 2, view->fontsizemedium, 14);
    style_box_translate(&ihandle->box, view->marginw * 2, view->marginh * 2 + 4);

    if (payload->mode.mode == ALFI_MODE_ON)
        style_box_translate(&ihandle->box, 2 * view->unitw - 2 * view->marginw, 0);

    if (widget->header.state == ALFI_STATE_FOCUS)
        style_color_clone(&ohandle->color, &color_focus);
    else
        style_color_clone(&ohandle->color, &color_text);

    style_box_init(&ohandle->box, x, y, view->unitw * 2 - view->unitw / 2, view->fontsizemedium, 16);
    style_box_translate(&ohandle->box, view->marginw * 2, view->marginh * 2 + 4);

    if (payload->mode.mode == ALFI_MODE_ON)
        style_box_translate(&ohandle->box, 2 * view->unitw - 2 * view->marginw, 0);

    style_font_init(&text->font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, 3 * view->unitw + view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    if (payload->mode.mode == ALFI_MODE_ON)
        style_color_clone(&groove->color, &color_focus);
    else
        style_color_clone(&groove->color, &color_line);

    style_box_init(&groove->box, x, y, 2 * view->unitw, view->fontsizemedium, 8);
    style_box_translate(&groove->box, view->marginw, view->marginh);
    style_box_shrink(&groove->box, 4, 4);

    return text->box.h + view->marginh * 2;

}

static void toggle_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_toggle *payload = &widget->payload.toggle;
    struct style *groove = &frame->styles[0];
    struct style *text = &frame->styles[1];
    struct style *ohandle = &frame->styles[2];
    struct style *ihandle = &frame->styles[3];

    render_fillrect(groove);
    render_fillcircle(ohandle);
    render_fillcircle(ihandle);

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int toggle_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
    case ALFI_STATE_UNHOVER:
        if (widget->header.state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;

    }

    return state;

}

static unsigned int toggle_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *groove = &frame->styles[0];
    struct style *text = &frame->styles[1];

    if (style_box_istouching(&groove->box, x, y))
        return ALFI_CURSOR_HAND;
    else if (style_box_istouching(&text->box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void stack_create(struct widget *widget)
{

}

static void stack_destroy(struct widget *widget)
{

}

static int stack_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget *child = 0;
    int cx = x;
    int cy = y;
    int cw = w;
    int h = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = call_animate(child, cx, cy, cw, view, u);

        if (h < cy + ch - y)
            h = cy + ch - y;

        cy += ch;

    }

    return h;

}

static void stack_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
        call_render(child, view);

}

static unsigned int stack_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int stack_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

static void window_create(struct widget *widget)
{

    struct payload_window *payload = &widget->payload.window;

    payload->label.content = pool_string_create(ALFI_ATTRIBUTE_LABEL, payload->label.content, "undefined");

}

static void window_destroy(struct widget *widget)
{

    struct payload_window *payload = &widget->payload.window;

    payload->label.content = pool_string_destroy(ALFI_ATTRIBUTE_LABEL, payload->label.content);

}

static int window_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget *child = 0;
    int pw = view->unitw * 2;
    int ph = view->unith * 2;
    int cx = x + pw;
    int cy = y + ph;
    int cw = w - pw * 2;
    int h = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = call_animate(child, cx, cy, cw, view, u);

        if (h < cy + ch + ph - y)
            h = cy + ch + ph - y;

    }

    style_color_clone(&frame->styles[0].color, &color_background);

    return h;

}

static void window_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
        call_render(child, view);

}

static unsigned int window_setstate(struct widget *widget, unsigned int state)
{

    return state;

}

static unsigned int window_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

void widgets_setupfonts(void)
{

    render_loadfont(&res_font_regular, "file:///usr/share/navi/roboto-regular.ttf");
    render_loadfont(&res_font_bold, "file:///usr/share/navi/roboto-bold.ttf");
    render_loadfont(&res_font_mono, "file:///usr/share/navi/robotomono-regular.ttf");
    render_loadfont(&res_font_icon, "file:///usr/share/navi/icofont.ttf");

}

void widgets_settheme(unsigned int type)
{

    switch (type)
    {

    case ALFI_THEME_LIGHT:
        style_color_init(&color_background, 255, 255, 255, 255);
        style_color_init(&color_text, 96, 96, 96, 255);
        style_color_init(&color_header, 64, 64, 64, 255);
        style_color_init(&color_focus, 96, 192, 192, 255);
        style_color_init(&color_focustext, 255, 255, 255, 255);
        style_color_init(&color_line, 192, 192, 192, 255);

        break;

    case ALFI_THEME_DARK:
        style_color_init(&color_background, 24, 24, 24, 255);
        style_color_init(&color_text, 192, 192, 192, 255);
        style_color_init(&color_header, 224, 224, 224, 255);
        style_color_init(&color_focus, 96, 192, 192, 255);
        style_color_init(&color_focustext, 255, 255, 255, 255);
        style_color_init(&color_line, 96, 96, 96, 255);

        break;

    }

}

void widgets_setup(void)
{

    call_register(ALFI_WIDGET_ANCHOR, ALFI_FLAG_NONE, anchor_create, anchor_destroy, anchor_animate, anchor_render, anchor_setstate, anchor_getcursor);
    call_register(ALFI_WIDGET_BUTTON, ALFI_FLAG_FOCUSABLE, button_create, button_destroy, button_animate, button_render, button_setstate, button_getcursor);
    call_register(ALFI_WIDGET_CHOICE, ALFI_FLAG_NONE, choice_create, choice_destroy, choice_animate, choice_render, choice_setstate, choice_getcursor);
    call_register(ALFI_WIDGET_DIVIDER, ALFI_FLAG_NONE, divider_create, divider_destroy, divider_animate, divider_render, divider_setstate, divider_getcursor);
    call_register(ALFI_WIDGET_FIELD, ALFI_FLAG_FOCUSABLE, field_create, field_destroy, field_animate, field_render, field_setstate, field_getcursor);
    call_register(ALFI_WIDGET_HEADER, ALFI_FLAG_NONE, header_create, header_destroy, header_animate, header_render, header_setstate, header_getcursor);
    call_register(ALFI_WIDGET_IMAGE, ALFI_FLAG_NONE, image_create, image_destroy, image_animate, image_render, image_setstate, image_getcursor);
    call_register(ALFI_WIDGET_LIST, ALFI_FLAG_NONE, list_create, list_destroy, list_animate, list_render, list_setstate, list_getcursor);
    call_register(ALFI_WIDGET_SELECT, ALFI_FLAG_FOCUSABLE, select_create, select_destroy, select_animate, select_render, select_setstate, select_getcursor);
    call_register(ALFI_WIDGET_STACK, ALFI_FLAG_NONE, stack_create, stack_destroy, stack_animate, stack_render, stack_setstate, stack_getcursor);
    call_register(ALFI_WIDGET_SUBHEADER, ALFI_FLAG_NONE, subheader_create, subheader_destroy, subheader_animate, subheader_render, subheader_setstate, subheader_getcursor);
    call_register(ALFI_WIDGET_TABLE, ALFI_FLAG_NONE, table_create, table_destroy, table_animate, table_render, table_setstate, table_getcursor);
    call_register(ALFI_WIDGET_TEXT, ALFI_FLAG_NONE, text_create, text_destroy, text_animate, text_render, text_setstate, text_getcursor);
    call_register(ALFI_WIDGET_TOGGLE, ALFI_FLAG_FOCUSABLE, toggle_create, toggle_destroy, toggle_animate, toggle_render, toggle_setstate, toggle_getcursor);
    call_register(ALFI_WIDGET_WINDOW, ALFI_FLAG_NONE, window_create, window_destroy, window_animate, window_render, window_setstate, window_getcursor);

}

