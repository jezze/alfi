#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "gridfmt.h"
#include "attributes.h"
#include "widgets.h"
#include "entity.h"
#include "animation.h"
#include "pool.h"
#include "render.h"

struct call
{

    int (*step)(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u);
    void (*render)(struct widget *widget, struct frame *frame, struct view *view);
    unsigned int (*getcursor)(struct widget *widget, struct frame *frame, int x, int y);

};

static struct call calls[64];
static struct resource *font_regular;
static struct resource *font_bold;
static struct resource *font_mono;
static struct resource *font_icon;
static struct style_color color_background;
static struct style_color color_text;
static struct style_color color_header;
static struct style_color color_focus;
static struct style_color color_focustext;
static struct style_color color_line;

static int anchor_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_anchor *payload = &widget->payload.anchor;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_focus);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void anchor_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_anchor *payload = &widget->payload.anchor;
    struct style *text = &frame->styles[0];

    render_filltext(text, payload->label.content);

}

static unsigned int anchor_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int button_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_button *payload = &widget->payload.button;
    struct style *surface = &frame->styles[0];
    struct style *text = &frame->styles[1];

    style_font_init(&text->font, font_bold->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_color_clone(&text->color, &color_focustext);
    else
        style_color_clone(&text->color, &color_text);

    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, 3 * view->marginw, 3 * view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
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

    struct widget_payload_button *payload = &widget->payload.button;
    struct style *surface = &frame->styles[0];
    struct style *text = &frame->styles[1];

    render_fillrect(surface);

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int button_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *surface = &frame->styles[0];

    if (style_box_istouching(&surface->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int choice_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_choice *payload = &widget->payload.choice;
    struct style *background = &frame->styles[0];
    struct style *text = &frame->styles[1];

    style_font_init(&text->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    if (widget->header.state == WIDGET_STATE_HOVER)
        style_color_clone(&background->color, &color_line);

    style_box_init(&background->box, x, y, w, 0, 4);
    style_box_expand(&background->box, &text->box, view->marginw, view->marginh);

    return background->box.h;

}

static void choice_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_choice *payload = &widget->payload.choice;
    struct style *background = &frame->styles[0];
    struct style *text = &frame->styles[1];

    if (widget->header.state == WIDGET_STATE_HOVER)
        render_fillrect(background);

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int choice_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ANIMATION_CURSOR_HAND;

}

static int code_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_code *payload = &widget->payload.code;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, font_mono->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void code_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_code *payload = &widget->payload.code;
    struct style *text = &frame->styles[0];

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int code_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int divider_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_divider *payload = &widget->payload.divider;
    struct style *line = &frame->styles[0];
    struct style *text = &frame->styles[1];

    style_font_init(&text->font, font_regular->index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
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

    struct widget_payload_divider *payload = &widget->payload.divider;
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

static unsigned int divider_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[1];

    if (style_box_istouching(&text->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int field_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_field *payload = &widget->payload.field;
    struct style *border = &frame->styles[0];
    struct style *label = &frame->styles[1];
    struct style *data = &frame->styles[2];

    style_font_init(&data->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&data->color, &color_text);
    style_box_init(&data->box, x, y, w, 0, 0);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_box_shrink(&data->box, view->unitw + view->marginw, view->unith + view->marginh);
    else
        style_box_shrink(&data->box, view->unitw, view->unith);

    style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));
    style_box_init(&label->box, x, y, w, 0, 0);

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
        style_font_init(&label->font, font_regular->index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    else
        style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&label->color, &color_focus);
    else
        style_color_clone(&label->color, &color_line);

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
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

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&border->color, &color_focus);
    else
        style_color_clone(&border->color, &color_line);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_box_expand(&border->box, &data->box, view->unitw, view->unith);
    else
        style_box_expand(&border->box, &data->box, view->unitw - view->marginw, view->unith - view->marginh);

    return border->box.h + view->marginh * 2;

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

        if (strlen(payload->label.content))
            render_filltext(label, payload->label.content);

    }

    if (widget->header.state == WIDGET_STATE_FOCUS)
        render_filltextinput(data, payload->data.content, payload->data.offset, &border->color);
    else
        render_filltext(data, payload->data.content);

}

static unsigned int field_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *data = &frame->styles[2];

    if (style_box_istouching(&data->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int header_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_header *payload = &widget->payload.header;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, font_bold->index, view->fontsizexlarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_header);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void header_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_header *payload = &widget->payload.header;
    struct style *text = &frame->styles[0];

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int header_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int image_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_image *payload = &widget->payload.image;
    struct resource *resource = pool_resource_find(payload->link.url);
    struct style *surface = &frame->styles[0];
    float ratio;

    if (!resource)
        return 0;

    ratio = (float)w / (float)resource->w;

    style_box_init(&surface->box, x, y, resource->w * ratio, resource->h * ratio, 0);
    style_box_shrink(&surface->box, view->marginw, view->marginh);

    /* Fixed size */
    /*
    style_box_init(&surface->box, x, y, resource->w, resource->h, 0);
    style_box_translate(&surface->box, view->marginw, view->marginh);
    */

    return surface->box.h + view->marginh * 2;

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

static unsigned int image_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ANIMATION_CURSOR_ARROW;

}

static int list_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget *child = 0;
    int pw = view->unitw * 1;
    int cx = x + pw;
    int cy = y;
    int cw = w - pw * 2;
    int h = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        int ch = animation_step(child, cx, cy, cw, view, u);

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

        animation_render(child, view);

        if (child->header.type != WIDGET_TYPE_LIST)
        {

            struct style dot;

            style_init(&dot);
            style_color_clone(&dot.color, &color_header);
            style_box_init(&dot.box, child->frame.bounds.x - view->unitw / 2 + view->marginw, child->frame.bounds.y + child->frame.bounds.h / 2, 0, 0, 3);
            render_fillcircle(&dot);

        }

    }

}

static unsigned int list_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ANIMATION_CURSOR_ARROW;

}

static int select_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_select *payload = &widget->payload.select;
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

        int ch = animation_step(child, x + cx, y + cy, cw, view, u);

        cy += ch;

        if (widget->header.state == WIDGET_STATE_FOCUS)
        {

            if (h < cy + view->unith)
                h = cy + view->unith;

        }

    }

    style_font_init(&data->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&data->color, &color_text);
    style_box_init(&data->box, x, y, w, h, 0);

    if (widget->header.state == WIDGET_STATE_FOCUS)
    {

        style_box_shrink(&data->box, view->unitw + view->marginw, view->unith + view->marginh);
        style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));

    }

    else
    {

        style_box_shrink(&data->box, view->unitw, view->unith);
        style_box_scale(&data->box, data->box.w, render_textheight(data, payload->data.content));

    }

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
        style_font_init(&label->font, font_regular->index, view->fontsizesmall, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    else
        style_font_init(&label->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&label->color, &color_focus);
    else
        style_color_clone(&label->color, &color_line);

    style_box_init(&label->box, x, y, w, h, 0);

    if (widget->header.state == WIDGET_STATE_FOCUS || strlen(payload->data.content))
    {

        style_box_translate(&label->box, view->unitw, 0);
        style_box_scale(&label->box, render_textwidth(label, payload->label.content), render_textheight(label, payload->label.content));

    }

    else
    {

        style_box_translate(&label->box, view->unitw, view->unith);
        style_box_scale(&label->box, label->box.w, render_textheight(label, payload->label.content));

    }

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&border->color, &color_focus);
    else
        style_color_clone(&border->color, &color_line);

    style_box_init(&border->box, x, y, w, h, 4);
    style_box_shrink(&border->box, view->marginw, view->marginh);

    return border->box.h + view->marginh * 2;

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

        if (strlen(payload->label.content))
            render_filltext(label, payload->label.content);

    }

    if (strlen(payload->data.content))
        render_filltext(data, payload->data.content);

    if (widget->header.state == WIDGET_STATE_FOCUS)
    {

        while ((child = pool_widget_nextchild(child, widget)))
            animation_render(child, view);

    }

}

static unsigned int select_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *border = &frame->styles[0];

    if (style_box_istouching(&border->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int subheader_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_subheader *payload = &widget->payload.subheader;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, font_bold->index, view->fontsizelarge, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_header);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void subheader_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_subheader *payload = &widget->payload.subheader;
    struct style *text = &frame->styles[0];

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int subheader_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int table_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_table *payload = &widget->payload.table;
    unsigned int gsize = gridfmt_size(payload->grid.format);
    struct widget *child = 0;
    unsigned int i;
    int cx = x;
    int cy = y;
    int h = 0;

    for (i = 0; (child = pool_widget_nextchild(child, widget)); i++)
    {

        unsigned int csize = gridfmt_colsize(payload->grid.format, i % gsize);
        int cw = csize * view->unitw * 2;
        int ch = animation_step(child, cx, cy, cw, view, u);

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
        animation_render(child, view);

}

static unsigned int table_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ANIMATION_CURSOR_ARROW;

}

static int text_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_text *payload = &widget->payload.text;
    struct style *text = &frame->styles[0];

    style_font_init(&text->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    return text->box.h + view->marginh * 2;

}

static void text_render(struct widget *widget, struct frame *frame, struct view *view)
{

    struct widget_payload_text *payload = &widget->payload.text;
    struct style *text = &frame->styles[0];

    if (strlen(payload->label.content))
        render_filltext(text, payload->label.content);

}

static unsigned int text_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *text = &frame->styles[0];

    if (style_box_istouching(&text->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int toggle_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
{

    struct widget_payload_toggle *payload = &widget->payload.toggle;
    struct style *groove = &frame->styles[0];
    struct style *text = &frame->styles[1];
    struct style *ohandle = &frame->styles[2];
    struct style *ihandle = &frame->styles[3];

    style_color_clone(&ihandle->color, &color_background);
    style_box_init(&ihandle->box, x, y, view->unitw * 2 - view->unitw / 2, view->fontsizemedium, 14);
    style_box_translate(&ihandle->box, view->marginw * 2, view->marginh * 2 + 4);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_box_translate(&ihandle->box, 2 * view->unitw - 2 * view->marginw, 0);

    if (widget->header.state == WIDGET_STATE_FOCUS)
        style_color_clone(&ohandle->color, &color_focus);
    else
        style_color_clone(&ohandle->color, &color_text);

    style_box_init(&ohandle->box, x, y, view->unitw * 2 - view->unitw / 2, view->fontsizemedium, 16);
    style_box_translate(&ohandle->box, view->marginw * 2, view->marginh * 2 + 4);

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
        style_box_translate(&ohandle->box, 2 * view->unitw - 2 * view->marginw, 0);

    style_font_init(&text->font, font_regular->index, view->fontsizemedium, STYLE_ALIGN_LEFT | STYLE_ALIGN_TOP);
    style_color_clone(&text->color, &color_text);
    style_box_init(&text->box, x, y, w, 0, 0);
    style_box_shrink(&text->box, 3 * view->unitw + view->marginw, view->marginh);
    style_box_scale(&text->box, render_textwidth(text, payload->label.content), render_textheight(text, payload->label.content));

    if (payload->mode.type == ATTRIBUTE_MODE_ON)
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

    struct widget_payload_toggle *payload = &widget->payload.toggle;
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

static unsigned int toggle_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    struct style *groove = &frame->styles[0];
    struct style *text = &frame->styles[1];

    if (style_box_istouching(&groove->box, x, y))
        return ANIMATION_CURSOR_HAND;
    else if (style_box_istouching(&text->box, x, y))
        return ANIMATION_CURSOR_IBEAM;
    else
        return ANIMATION_CURSOR_ARROW;

}

static int window_step(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u)
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

        int ch = animation_step(child, cx, cy, cw, view, u);

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
        animation_render(child, view);

}

static unsigned int window_getcursor(struct widget *widget, struct frame *frame, int x, int y)
{

    return ANIMATION_CURSOR_ARROW;

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

    if (style_box_compare(&frame->bounds, &keyframe->bounds))
        return 1;

    for (i = 0; i < 8; i++)
    {

        if (style_compare(&frame->styles[i], &keyframe->styles[i]))
            return 1;

    }

    return 0;

}

int animation_step(struct widget *widget, int x, int y, int w, struct view *view, float u)
{

    struct frame *frame = &widget->frame;
    struct frame keyframe;

    initframe(&keyframe, x, y, w);

    keyframe.bounds.h = calls[widget->header.type].step(widget, &keyframe, x, y, w, view, u);

    if (widget->header.type == WIDGET_TYPE_WINDOW)
        tweenframe(frame, &keyframe, 1.0);
    else
        tweenframe(frame, &keyframe, u);

    frame->animating = compareframe(frame, &keyframe);

    return frame->bounds.h;

}

void animation_render(struct widget *widget, struct view *view)
{

    struct frame *frame = &widget->frame;

    calls[widget->header.type].render(widget, frame, view);

}

unsigned int animation_getcursor(struct widget *widget, int x, int y)
{

    struct frame *frame = &widget->frame;

    return calls[widget->header.type].getcursor(widget, frame, x, y);

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
        style_color_init(&color_focustext, 255, 255, 255, 255);
        style_color_init(&color_line, 192, 192, 192, 255);

        break;

    case ANIMATION_THEME_DARK:
        style_color_init(&color_background, 24, 24, 24, 255);
        style_color_init(&color_text, 192, 192, 192, 255);
        style_color_init(&color_header, 224, 224, 224, 255);
        style_color_init(&color_focus, 96, 192, 192, 255);
        style_color_init(&color_focustext, 255, 255, 255, 255);
        style_color_init(&color_line, 96, 96, 96, 255);

        break;

    }

}

static void setcallback(unsigned int type, int (*step)(struct widget *widget, struct frame *frame, int x, int y, int w, struct view *view, float u), void (*render)(struct widget *widget, struct frame *frame, struct view *view), unsigned int (*getcursor)(struct widget *widget, struct frame *frame, int x, int y))
{

    calls[type].step = step;
    calls[type].render = render;
    calls[type].getcursor = getcursor;

}

void animation_setup(void)
{

    setcallback(WIDGET_TYPE_ANCHOR, anchor_step, anchor_render, anchor_getcursor);
    setcallback(WIDGET_TYPE_BUTTON, button_step, button_render, button_getcursor);
    setcallback(WIDGET_TYPE_CHOICE, choice_step, choice_render, choice_getcursor);
    setcallback(WIDGET_TYPE_CODE, code_step, code_render, code_getcursor);
    setcallback(WIDGET_TYPE_DIVIDER, divider_step, divider_render, divider_getcursor);
    setcallback(WIDGET_TYPE_FIELD, field_step, field_render, field_getcursor);
    setcallback(WIDGET_TYPE_HEADER, header_step, header_render, header_getcursor);
    setcallback(WIDGET_TYPE_IMAGE, image_step, image_render, image_getcursor);
    setcallback(WIDGET_TYPE_LIST, list_step, list_render, list_getcursor);
    setcallback(WIDGET_TYPE_SELECT, select_step, select_render, select_getcursor);
    setcallback(WIDGET_TYPE_SUBHEADER, subheader_step, subheader_render, subheader_getcursor);
    setcallback(WIDGET_TYPE_TABLE, table_step, table_render, table_getcursor);
    setcallback(WIDGET_TYPE_TEXT, text_step, text_render, text_getcursor);
    setcallback(WIDGET_TYPE_TOGGLE, toggle_step, toggle_render, toggle_getcursor);
    setcallback(WIDGET_TYPE_WINDOW, window_step, window_render, window_getcursor);

}

