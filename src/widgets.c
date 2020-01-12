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

    payload->label.content = pool_string_create(payload->label.content, "");
    payload->link.url = pool_string_create(payload->link.url, "");
    payload->link.mime = pool_string_create(payload->link.mime, "");

}

static void anchor_destroy(struct widget *widget)
{

    struct payload_anchor *payload = &widget->payload.anchor;

    payload->label.content = pool_string_destroy(payload->label.content);
    payload->link.url = pool_string_destroy(payload->link.url);
    payload->link.mime = pool_string_destroy(payload->link.mime);

}

static int anchor_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_anchor *payload = &widget->payload.anchor;

    style_font_init(&frame->styles[0].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&frame->styles[0].color, &color_focus);
    style_box_init(&frame->styles[0].box, x, y, w, 0, 0);
    style_box_pad(&frame->styles[0].box, view->marginw, view->marginh);
    style_box_scale(&frame->styles[0].box, render_textwidth(&frame->styles[0], payload->label.content), render_textheight(&frame->styles[0], payload->label.content));

    return frame->styles[0].box.h + view->marginh * 2;

}

static void anchor_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_anchor *payload = &widget->payload.anchor;

    render_filltext(&frame->styles[0], payload->label.content);

}

static unsigned int anchor_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int anchor_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[0].box, x, y))
        return ALFI_CURSOR_HAND;
    else
        return ALFI_CURSOR_ARROW;

}

static void button_create(struct widget *widget)
{

    struct payload_button *payload = &widget->payload.button;

    payload->label.content = pool_string_create(payload->label.content, "undefined");
    payload->link.url = pool_string_create(payload->link.url, "");
    payload->link.mime = pool_string_create(payload->link.mime, "");

}

static void button_destroy(struct widget *widget)
{

    struct payload_button *payload = &widget->payload.button;

    payload->label.content = pool_string_destroy(payload->label.content);
    payload->link.url = pool_string_destroy(payload->link.url);
    payload->link.mime = pool_string_destroy(payload->link.mime);

}

static int button_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_button *payload = &widget->payload.button;

    style_font_init(&frame->styles[2].font, res_font_bold.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (payload->mode.mode == ALFI_MODE_ON)
    {

        style_color_clone(&frame->styles[1].color, &color_focus);
        style_color_clone(&frame->styles[2].color, &color_focustext);

    }

    else
    {

        style_color_clone(&frame->styles[1].color, &color_line);
        style_color_clone(&frame->styles[2].color, &color_text);

    }

    style_box_init(&frame->styles[0].box, x, y, w, view->unith * 3, 4);
    style_box_clone(&frame->styles[1].box, &frame->styles[0].box);
    style_box_pad(&frame->styles[1].box, view->marginw, view->marginh);
    style_box_clone(&frame->styles[2].box, &frame->styles[1].box);
    style_box_pad(&frame->styles[2].box, view->unitw - view->marginw, view->unith - view->marginh);
    style_box_scale(&frame->styles[2].box, render_textwidth(&frame->styles[2], payload->label.content), render_textheight(&frame->styles[2], payload->label.content));
    style_box_expand(&frame->styles[1].box, &frame->styles[2].box, view->unitw - view->marginw, view->unith - view->marginh);
    style_box_expand(&frame->styles[0].box, &frame->styles[1].box, view->marginw, view->marginh);

    return frame->styles[0].box.h;

}

static void button_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_button *payload = &widget->payload.button;

    render_fillrect(&frame->styles[1]);

    if (strlen(payload->label.content))
        render_filltext(&frame->styles[2], payload->label.content);

}

static unsigned int button_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_HOVER;

    case ALFI_STATE_UNHOVER:
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_NORMAL;

    case ALFI_STATE_FOCUS:
        return ALFI_STATE_FOCUS;

    }

    return ALFI_STATE_NORMAL;

}

static unsigned int button_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[1].box, x, y))
        return ALFI_CURSOR_HAND;
    else
        return ALFI_CURSOR_ARROW;

}

static void choice_create(struct widget *widget)
{

    struct payload_choice *payload = &widget->payload.choice;

    payload->label.content = pool_string_create(payload->label.content, "");

}

static void choice_destroy(struct widget *widget)
{

    struct payload_choice *payload = &widget->payload.choice;

    payload->label.content = pool_string_destroy(payload->label.content);

}

static int choice_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_choice *payload = &widget->payload.choice;

    style_font_init(&frame->styles[1].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&frame->styles[1].color, &color_text);

    if (widget->state == ALFI_STATE_HOVER)
        style_color_clone(&frame->styles[0].color, &color_line);

    style_box_init(&frame->styles[0].box, x, y, w, 0, 4);
    style_box_clone(&frame->styles[1].box, &frame->styles[0].box);
    style_box_pad(&frame->styles[1].box, view->marginw, view->marginh);
    style_box_scale(&frame->styles[1].box, render_textwidth(&frame->styles[1], payload->label.content), render_textheight(&frame->styles[1], payload->label.content));
    style_box_expand(&frame->styles[0].box, &frame->styles[1].box, view->marginw, view->marginh);

    return frame->styles[0].box.h;

}

static void choice_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_choice *payload = &widget->payload.choice;

    if (widget->state == ALFI_STATE_HOVER)
        render_fillrect(&frame->styles[0]);

    if (strlen(payload->label.content))
        render_filltext(&frame->styles[1], payload->label.content);

}

static unsigned int choice_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
        return ALFI_STATE_HOVER;

    case ALFI_STATE_UNHOVER:
        return ALFI_STATE_NORMAL;

    }

    return ALFI_STATE_NORMAL;

}

static unsigned int choice_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_HAND;

}

static void divider_create(struct widget *widget)
{

    struct payload_divider *payload = &widget->payload.divider;

    payload->label.content = pool_string_create(payload->label.content, "");

}

static void divider_destroy(struct widget *widget)
{

    struct payload_divider *payload = &widget->payload.divider;

    payload->label.content = pool_string_destroy(payload->label.content);

}

static int divider_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_divider *payload = &widget->payload.divider;

    style_font_init(&frame->styles[2].font, res_font_regular.index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&frame->styles[2].color, &color_text);
    style_color_clone(&frame->styles[1].color, &color_line);
    style_box_init(&frame->styles[0].box, x, y, w, view->unith * 2, 0);
    style_box_init(&frame->styles[1].box, x + view->marginw, y + view->unith - 1, w - view->marginw * 2, 2, 0);
    style_box_init(&frame->styles[2].box, x, y, w, 0, 0);

    if (strlen(payload->label.content))
    {

        style_box_scale(&frame->styles[2].box, render_textwidth(&frame->styles[2], payload->label.content), render_textheight(&frame->styles[2], payload->label.content));
        style_box_translate(&frame->styles[2].box, w / 2 - frame->styles[2].box.w / 2, view->unith / 2 + 2);

    }

    return frame->styles[0].box.h;

}

static void divider_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_divider *payload = &widget->payload.divider;

    if (strlen(payload->label.content))
    {

        render_fillrectgap(&frame->styles[1], frame->styles[2].box.x - view->marginw, frame->styles[2].box.w + view->marginw * 2);
        render_filltext(&frame->styles[2], payload->label.content);

    }

    else
    {

        render_fillrect(&frame->styles[1]);

    }

}

static unsigned int divider_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int divider_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[2].box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void field_create(struct widget *widget)
{

    struct payload_field *payload = &widget->payload.field;

    payload->label.content = pool_string_create(payload->label.content, "");
    payload->data.content = pool_allocate(payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void field_destroy(struct widget *widget)
{

    struct payload_field *payload = &widget->payload.field;

    payload->label.content = pool_string_destroy(payload->label.content);
    payload->data.content = pool_string_destroy(payload->data.content);

}

static int field_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_field *payload = &widget->payload.field;

    if (widget->state == ALFI_STATE_FOCUS || strlen(payload->data.content))
        style_font_init(&frame->styles[2].font, res_font_regular.index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    else
        style_font_init(&frame->styles[2].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    style_font_init(&frame->styles[3].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (widget->state == ALFI_STATE_FOCUS)
        style_color_clone(&frame->styles[2].color, &color_focus);
    else
        style_color_clone(&frame->styles[2].color, &color_line);

    if (widget->state == ALFI_STATE_FOCUS)
        style_color_clone(&frame->styles[1].color, &color_focus);
    else
        style_color_clone(&frame->styles[1].color, &color_line);

    style_color_clone(&frame->styles[3].color, &color_text);
    style_box_init(&frame->styles[0].box, x, y, w, 0, 4);
    style_box_clone(&frame->styles[1].box, &frame->styles[0].box);
    style_box_clone(&frame->styles[2].box, &frame->styles[0].box);
    style_box_clone(&frame->styles[3].box, &frame->styles[0].box);
    style_box_pad(&frame->styles[1].box, view->marginw, view->marginh);

    if (widget->state == ALFI_STATE_FOCUS || strlen(payload->data.content))
    {

        style_box_pad(&frame->styles[2].box, view->unitw, view->marginh - frame->styles[2].font.size / 2);
        style_box_scale(&frame->styles[2].box, render_textwidth(&frame->styles[2], payload->label.content), render_textheight(&frame->styles[2], payload->label.content));

    }

    else
    {

        style_box_pad(&frame->styles[2].box, view->unitw, view->unith);
        style_box_scale(&frame->styles[2].box, frame->styles[2].box.w, render_textheight(&frame->styles[2], payload->label.content));

    }

    if (widget->state == ALFI_STATE_FOCUS)
        style_box_pad(&frame->styles[3].box, view->unitw + view->marginw, view->unith + view->marginh);
    else
        style_box_pad(&frame->styles[3].box, view->unitw, view->unith);

    style_box_scale(&frame->styles[3].box, frame->styles[3].box.w, render_textheight(&frame->styles[3], payload->data.content));

    if (widget->state == ALFI_STATE_FOCUS)
        style_box_expand(&frame->styles[1].box, &frame->styles[3].box, view->unitw, view->unith);
    else
        style_box_expand(&frame->styles[1].box, &frame->styles[3].box, view->unitw - view->marginw, view->unith - view->marginh);

    style_box_expand(&frame->styles[0].box, &frame->styles[1].box, view->marginw, view->marginh);

    return frame->styles[0].box.h;

}

static void field_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_field *payload = &widget->payload.field;

    if (widget->state == ALFI_STATE_FOCUS || strlen(payload->data.content))
    {

        if (strlen(payload->label.content))
        {

            render_fillbordergap(&frame->styles[1], 2.0, frame->styles[2].box.x - view->marginw, frame->styles[2].box.w + view->marginw * 2);
            render_filltext(&frame->styles[2], payload->label.content);

        }

        else
        {

            render_fillborder(&frame->styles[1], 2.0);

        }

    }

    else
    {

        render_fillborder(&frame->styles[1], 2.0);

        if (strlen(payload->label.content))
            render_filltext(&frame->styles[2], payload->label.content);

    }

    if (strlen(payload->data.content))
    {

        if (widget->state == ALFI_STATE_FOCUS)
            render_filltextinput(&frame->styles[3], payload->data.content, payload->data.offset, &frame->styles[1].color);
        else
            render_filltext(&frame->styles[3], payload->data.content);

    }

}

static unsigned int field_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_HOVER;

    case ALFI_STATE_UNHOVER:
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_NORMAL;

    case ALFI_STATE_FOCUS:
        return ALFI_STATE_FOCUS;

    }

    return ALFI_STATE_NORMAL;

}

static unsigned int field_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[3].box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void header_create(struct widget *widget)
{

    struct payload_header *payload = &widget->payload.header;

    payload->label.content = pool_string_create(payload->label.content, "");

}

static void header_destroy(struct widget *widget)
{

    struct payload_header *payload = &widget->payload.header;

    payload->label.content = pool_string_destroy(payload->label.content);

}

static int header_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_header *payload = &widget->payload.header;

    style_font_init(&frame->styles[0].font, res_font_bold.index, view->fontsizexlarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&frame->styles[0].color, &color_header);
    style_box_init(&frame->styles[0].box, x, y, w, 0, 0);
    style_box_pad(&frame->styles[0].box, view->marginw, view->marginh);
    style_box_scale(&frame->styles[0].box, render_textwidth(&frame->styles[0], payload->label.content), render_textheight(&frame->styles[0], payload->label.content));

    return frame->styles[0].box.h + view->marginh * 2;

}

static void header_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_header *payload = &widget->payload.header;

    if (strlen(payload->label.content))
        render_filltext(&frame->styles[0], payload->label.content);

}

static unsigned int header_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int header_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[0].box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void hstack_create(struct widget *widget)
{

}

static void hstack_destroy(struct widget *widget)
{

}

static int hstack_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget *child = 0;
    int cx = x;
    int cy = y;
    int cw = 0;
    int h = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = call_animate(child, cx, cy, cw, view, u);

        if (h < cy + ch - y)
            h = cy + ch - y;

        cx += child->frame.bounds.w;

    }

    return h;

}

static void hstack_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
        call_render(child, view);

}

static unsigned int hstack_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int hstack_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

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

    style_box_init(&frame->styles[1].box, x, y, resource->w, resource->h, 0);
    style_box_translate(&frame->styles[1].box, view->marginw, view->marginh);
    style_box_clone(&frame->styles[0].box, &frame->styles[1].box);
    style_box_pad(&frame->styles[0].box, -((int)view->marginw), -((int)view->marginh));

    return frame->styles[0].box.h;

}

static void image_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct resource_image *resource = &widget->resource.image;

    render_fillimage(&frame->styles[1], resource->ref);

}

static unsigned int image_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

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

        cy += child->frame.bounds.h;

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

            dot.box.r = 3.0;
            dot.box.x = child->frame.bounds.x - view->unitw / 2 + view->marginw;
            dot.box.y = child->frame.bounds.y + child->frame.bounds.h / 2;

            render_fillcircle(&dot);

        }

    }

}

static unsigned int list_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int list_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

static void select_create(struct widget *widget)
{

    struct payload_select *payload = &widget->payload.select;

    payload->label.content = pool_string_create(payload->label.content, "");
    payload->data.content = pool_allocate(payload->data.content, ALFI_DATASIZE, 1, "");
    payload->data.total = ALFI_DATASIZE;
    payload->data.offset = 0;

}

static void select_destroy(struct widget *widget)
{

    struct payload_select *payload = &widget->payload.select;

    payload->label.content = pool_string_destroy(payload->label.content);
    payload->data.content = pool_string_destroy(payload->data.content);

}

static int select_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_select *payload = &widget->payload.select;
    struct widget *child = 0;
    int cx = view->unitw;
    int cy = view->unith * 3;
    int cw = w - view->unitw * 2;
    int h = view->unith * 3;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = call_animate(child, x + cx, y + cy, cw, view, u);

        cy += ch;

        if (widget->state == ALFI_STATE_FOCUS)
        {

            if (h < cy + view->unith)
                h = cy + view->unith;

        }

    }

    switch (widget->state)
    {

    case ALFI_STATE_FOCUS:
        style_font_init(&frame->styles[2].font, res_font_regular.index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
        style_box_init(&frame->styles[2].box, x, y, w, h, 0);
        style_box_translate(&frame->styles[2].box, view->unitw, 0);
        style_box_scale(&frame->styles[2].box, render_textwidth(&frame->styles[2], payload->label.content), render_textheight(&frame->styles[2], payload->label.content));
        style_color_clone(&frame->styles[2].color, &color_focus);
        style_font_init(&frame->styles[3].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
        style_box_init(&frame->styles[3].box, x, y, w, h, 0);
        style_box_pad(&frame->styles[3].box, view->unitw + view->marginw, view->unith + view->marginh);
        style_box_scale(&frame->styles[3].box, frame->styles[3].box.w, render_textheight(&frame->styles[3], payload->data.content));
        style_color_clone(&frame->styles[3].color, &color_text);
        style_box_init(&frame->styles[1].box, x, y, w, h, 4);
        style_box_pad(&frame->styles[1].box, view->marginw, view->marginh);
        style_color_clone(&frame->styles[1].color, &color_focus);
        style_box_init(&frame->styles[0].box, x, y, w, h, 0);

        break;

    default:
        if (strlen(payload->data.content))
        {

            style_font_init(&frame->styles[2].font, res_font_regular.index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
            style_box_init(&frame->styles[2].box, x, y, w, h, 0);
            style_box_translate(&frame->styles[2].box, view->unitw, 0);
            style_box_scale(&frame->styles[2].box, render_textwidth(&frame->styles[2], payload->label.content), render_textheight(&frame->styles[2], payload->label.content));
            style_color_clone(&frame->styles[2].color, &color_line);

        }

        else
        {

            style_font_init(&frame->styles[2].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
            style_box_init(&frame->styles[2].box, x, y, w, h, 0);
            style_box_translate(&frame->styles[2].box, view->unitw, view->unith);
            style_box_scale(&frame->styles[2].box, frame->styles[2].box.w, render_textheight(&frame->styles[2], payload->label.content));
            style_color_clone(&frame->styles[2].color, &color_line);

        }

        style_font_init(&frame->styles[3].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
        style_box_init(&frame->styles[3].box, x, y, w, h, 0);
        style_box_pad(&frame->styles[3].box, view->unitw, view->unith);
        style_box_scale(&frame->styles[3].box, frame->styles[3].box.w, render_textheight(&frame->styles[3], payload->data.content));
        style_color_clone(&frame->styles[3].color, &color_text);
        style_box_init(&frame->styles[1].box, x, y, w, h, 4);
        style_box_pad(&frame->styles[1].box, view->marginw, view->marginh);
        style_color_clone(&frame->styles[1].color, &color_line);
        style_box_init(&frame->styles[0].box, x, y, w, h, 0);

        break;

    }

    return frame->styles[0].box.h;

}

static void select_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_select *payload = &widget->payload.select;
    struct widget *child = 0;

    if (widget->state == ALFI_STATE_FOCUS || strlen(payload->data.content))
    {

        if (strlen(payload->label.content))
        {

            render_fillbordergap(&frame->styles[1], 2.0, frame->styles[2].box.x - view->marginw, frame->styles[2].box.w + view->marginw * 2);
            render_filltext(&frame->styles[2], payload->label.content);

        }

        else
        {

            render_fillborder(&frame->styles[1], 2.0);

        }

    }

    else
    {

        render_fillborder(&frame->styles[1], 2.0);

        if (strlen(payload->label.content))
            render_filltext(&frame->styles[2], payload->label.content);

    }

    if (strlen(payload->data.content))
        render_filltext(&frame->styles[3], payload->data.content);

    if (widget->state == ALFI_STATE_FOCUS)
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
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_HOVER;

    case ALFI_STATE_UNHOVER:
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_NORMAL;

    case ALFI_STATE_FOCUS:
        return ALFI_STATE_FOCUS;

    }

    return ALFI_STATE_NORMAL;

}

static unsigned int select_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[1].box, x, y))
        return ALFI_CURSOR_HAND;
    else
        return ALFI_CURSOR_ARROW;

}

static void subheader_create(struct widget *widget)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    payload->label.content = pool_string_create(payload->label.content, "");

}

static void subheader_destroy(struct widget *widget)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    payload->label.content = pool_string_destroy(payload->label.content);

}

static int subheader_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    style_font_init(&frame->styles[0].font, res_font_bold.index, view->fontsizelarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&frame->styles[0].color, &color_header);
    style_box_init(&frame->styles[0].box, x, y, w, 0, 0);
    style_box_pad(&frame->styles[0].box, view->marginw, view->marginh);
    style_box_scale(&frame->styles[0].box, render_textwidth(&frame->styles[0], payload->label.content), render_textheight(&frame->styles[0], payload->label.content));

    return frame->styles[0].box.h + view->marginh * 2;

}

static void subheader_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_subheader *payload = &widget->payload.subheader;

    if (strlen(payload->label.content))
        render_filltext(&frame->styles[0], payload->label.content);

}

static unsigned int subheader_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int subheader_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[0].box, x, y))
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

    return ALFI_STATE_NORMAL;

}

static unsigned int table_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

static void text_create(struct widget *widget)
{

    struct payload_text *payload = &widget->payload.text;

    payload->label.content = pool_string_create(payload->label.content, "");

}

static void text_destroy(struct widget *widget)
{

    struct payload_text *payload = &widget->payload.text;

    payload->label.content = pool_string_destroy(payload->label.content);

}

static int text_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_text *payload = &widget->payload.text;

    style_font_init(&frame->styles[0].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&frame->styles[0].color, &color_text);
    style_box_init(&frame->styles[0].box, x, y, w, 0, 0);
    style_box_pad(&frame->styles[0].box, view->marginw, view->marginh);
    style_box_scale(&frame->styles[0].box, render_textwidth(&frame->styles[0], payload->label.content), render_textheight(&frame->styles[0], payload->label.content));

    return frame->styles[0].box.h + view->marginh * 2;

}

static void text_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_text *payload = &widget->payload.text;

    if (strlen(payload->label.content))
        render_filltext(&frame->styles[0], payload->label.content);

}

static unsigned int text_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int text_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[0].box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void toggle_create(struct widget *widget)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    payload->label.content = pool_string_create(payload->label.content, "");

}

static void toggle_destroy(struct widget *widget)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    payload->label.content = pool_string_destroy(payload->label.content);

}

static int toggle_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    style_font_init(&frame->styles[2].font, res_font_regular.index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&frame->styles[2].color, &color_text);
    style_box_init(&frame->styles[0].box, x, y, w, 0, 0);
    style_box_clone(&frame->styles[2].box, &frame->styles[0].box);
    style_box_pad(&frame->styles[2].box, view->unitw + view->marginw, view->marginh);
    style_box_translate(&frame->styles[2].box, view->unitw, 0);
    style_box_scale(&frame->styles[2].box, render_textwidth(&frame->styles[2], payload->label.content), render_textheight(&frame->styles[2], payload->label.content));
    style_box_init(&frame->styles[1].box, x, y, view->unitw * 2 - view->unitw / 2, view->fontsizemedium, 8);
    style_box_translate(&frame->styles[1].box, view->marginw, view->marginh);

    if (widget->state == ALFI_STATE_FOCUS)
        style_color_clone(&frame->styles[1].color, &color_focus);
    else
        style_color_clone(&frame->styles[1].color, &color_line);

    style_box_expand(&frame->styles[0].box, &frame->styles[2].box, view->marginw, view->marginh);

    return frame->styles[0].box.h;

}

static void toggle_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct payload_toggle *payload = &widget->payload.toggle;

    render_fillborder(&frame->styles[1], 2.0);

    if (payload->mode.mode == ALFI_MODE_ON)
    {

        struct style checkmark;

        style_init(&checkmark);
        style_color_clone(&checkmark.color, &color_focus);
        style_box_clone(&checkmark.box, &frame->styles[1].box);
        style_box_pad(&checkmark.box, 18, 6);
        style_box_translate(&checkmark.box, 12, 0);
        render_fillrect(&checkmark);

    }

    if (payload->mode.mode == ALFI_MODE_OFF)
    {

        struct style checkmark;

        style_init(&checkmark);
        style_color_clone(&checkmark.color, &color_line);
        style_box_clone(&checkmark.box, &frame->styles[1].box);
        style_box_pad(&checkmark.box, 18, 6);
        style_box_translate(&checkmark.box, -12, 0);
        render_fillrect(&checkmark);

    }


    if (strlen(payload->label.content))
        render_filltext(&frame->styles[2], payload->label.content);

}

static unsigned int toggle_setstate(struct widget *widget, unsigned int state)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_HOVER;

    case ALFI_STATE_UNHOVER:
        if (widget->state == ALFI_STATE_FOCUS)
            return ALFI_STATE_FOCUS;
        else
            return ALFI_STATE_NORMAL;

    case ALFI_STATE_FOCUS:
        return ALFI_STATE_FOCUS;

    }

    return ALFI_STATE_NORMAL;

}

static unsigned int toggle_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    if (style_box_istouching(&frame->styles[1].box, x, y))
        return ALFI_CURSOR_HAND;
    else if (style_box_istouching(&frame->styles[2].box, x, y))
        return ALFI_CURSOR_IBEAM;
    else
        return ALFI_CURSOR_ARROW;

}

static void vstack_create(struct widget *widget)
{

}

static void vstack_destroy(struct widget *widget)
{

}

static int vstack_animate(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
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

        cy += child->frame.bounds.h;

    }

    return h;

}

static void vstack_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
        call_render(child, view);

}

static unsigned int vstack_setstate(struct widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static unsigned int vstack_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ALFI_CURSOR_ARROW;

}

static void window_create(struct widget *widget)
{

    struct payload_window *payload = &widget->payload.window;

    payload->label.content = pool_string_create(payload->label.content, "undefined");

}

static void window_destroy(struct widget *widget)
{

    struct payload_window *payload = &widget->payload.window;

    payload->label.content = pool_string_destroy(payload->label.content);

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

    return ALFI_STATE_NORMAL;

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
    call_register(ALFI_WIDGET_HSTACK, ALFI_FLAG_NONE, hstack_create, hstack_destroy, hstack_animate, hstack_render, hstack_setstate, hstack_getcursor);
    call_register(ALFI_WIDGET_IMAGE, ALFI_FLAG_NONE, image_create, image_destroy, image_animate, image_render, image_setstate, image_getcursor);
    call_register(ALFI_WIDGET_LIST, ALFI_FLAG_NONE, list_create, list_destroy, list_animate, list_render, list_setstate, list_getcursor);
    call_register(ALFI_WIDGET_SELECT, ALFI_FLAG_FOCUSABLE, select_create, select_destroy, select_animate, select_render, select_setstate, select_getcursor);
    call_register(ALFI_WIDGET_SUBHEADER, ALFI_FLAG_NONE, subheader_create, subheader_destroy, subheader_animate, subheader_render, subheader_setstate, subheader_getcursor);
    call_register(ALFI_WIDGET_TABLE, ALFI_FLAG_NONE, table_create, table_destroy, table_animate, table_render, table_setstate, table_getcursor);
    call_register(ALFI_WIDGET_TEXT, ALFI_FLAG_NONE, text_create, text_destroy, text_animate, text_render, text_setstate, text_getcursor);
    call_register(ALFI_WIDGET_TOGGLE, ALFI_FLAG_FOCUSABLE, toggle_create, toggle_destroy, toggle_animate, toggle_render, toggle_setstate, toggle_getcursor);
    call_register(ALFI_WIDGET_VSTACK, ALFI_FLAG_NONE, vstack_create, vstack_destroy, vstack_animate, vstack_render, vstack_setstate, vstack_getcursor);
    call_register(ALFI_WIDGET_WINDOW, ALFI_FLAG_NONE, window_create, window_destroy, window_animate, window_render, window_setstate, window_getcursor);

}

