#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "nvg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nvg_gl.h"
#include "list.h"
#include "box.h"
#include "widgets.h"
#include "parser.h"
#include "call.h"
#include "pool.h"

static struct parser parser;
static float marginx = 8.0;
static float marginy = 8.0;
static float gridx = 24.0;
static float gridy = 24.0;
static int fontface_regular;
static int fontface_bold;
static int fontface_icon;
static float fontsize_small;
static float fontsize_medium;
static float fontsize_large;
static float fontsize_xlarge;
static GLFWcursor *cursor_arrow;
static GLFWcursor *cursor_ibeam;
static GLFWcursor *cursor_hand;
static double mx;
static double my;
static int scrollx;
static int scrolly;
static int winw;
static int winh;
static int fbw;
static int fbh;
static struct alfi_widget *hover;
static struct alfi_widget *focus;
static struct alfi_color color_fade1;
static struct alfi_color color_fade2;
static struct alfi_color color_background;
static struct alfi_color color_text;
static struct alfi_color color_header;
static struct alfi_color color_focus;
static struct alfi_color color_line;
static struct nvg_context *ctx;
static char *url_default = "file://";
static char *url_home = "http://blunder.se/";
static char *url_test = "file:///usr/share/navi/example.alfi";
static unsigned int ngroups;

static struct alfi_widget *findrecursively(struct alfi_widget *widget, float x, float y)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        if (box_istouching(&child->bb, x, y))
            return findrecursively(child, x, y);

    }

    return widget;

}

static struct alfi_widget *findwidget(float x, float y)
{

    struct alfi_widget *root = 0;

    while ((root = pool_nextingroupoftype(root, 0, ALFI_WIDGET_WINDOW)))
    {

        if (box_istouching(&root->bb, x, y))
            return findrecursively(root, x, y);

    }

    return 0;

}

static struct alfi_widget *parser_find(char *name, unsigned int group)
{

    return pool_findbyname(group, name);

}

static char *parser_createstring(unsigned int size)
{

    return malloc(size);

}

static void parser_destroystring(char *string)
{

    free(string);

}

static struct alfi_widget *parser_create(unsigned int type, unsigned int group, char *in)
{

    struct alfi_widget *widget = pool_create();

    widget->type = type;
    widget->group = group;
    widget->in.name = in;

    return widget;

}

static void parser_destroy(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        parser_destroy(child);

        child = 0;

    }

    call_destroy(widget);
    pool_destroy(widget);

}

static void parser_fail(unsigned int line)
{

    printf("Parsing failed on line %u\n", line);
    exit(EXIT_FAILURE);

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

static void loadurl(char *path, unsigned int group)
{

    int fd[2];
    pid_t pid;

    if (pipe(fd) < 0)
        return;

    pid = fork();

    if (pid < 0)
        return;

    if (pid)
    {

        char data[4096];
        int count;

        close(fd[1]);

        while ((count = read(fd[0], data, 4096)))
            parser_parsedata(&parser, group, "main", count, data);

        close(fd[0]);

    }

    else
    {

        dup2(fd[0], 0);
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execlp("navi-resolve", "navi-resolve", path, NULL);
        exit(EXIT_FAILURE);

    }

}

static void place(unsigned int group, float u)
{

    struct alfi_widget *widget = 0;
    int offx = 0;
    int offy = 0;

    while ((widget = pool_nextingroupoftype(widget, group, ALFI_WIDGET_WINDOW)))
    {

        call_place(widget, offx + scrollx, offy + scrolly, 0, 0, u);

        offx += widget->bb.w + gridx * 2;

    }

}

static void init(unsigned int group)
{

    struct alfi_widget *widget = 0;

    while ((widget = pool_nextingroup(widget, group)))
    {

        call_init(widget);
        call_setstate(widget, ALFI_STATE_NORMAL);

    }

}

static void createpage(char *url)
{

    ngroups++;

    loadbase(ngroups);
    loadurl(url, ngroups);
    init(ngroups);
    place(0, 1.0);

}

static void setfocus(struct alfi_widget *widget)
{

    if (widget != focus)
    {

        if (focus)
            call_setstate(focus, ALFI_STATE_UNFOCUS);

        focus = widget;

        if (focus)
            call_setstate(focus, ALFI_STATE_FOCUS);

    }

}

static void sethover(struct alfi_widget *widget)
{

    if (widget != hover)
    {

        if (hover)
            call_setstate(hover, ALFI_STATE_UNHOVER);

        hover = widget;

        if (hover)
            call_setstate(hover, ALFI_STATE_HOVER);

    }

}

static void alfi_style_fittext(struct alfi_style *style, char *text)
{

    struct nvg_textrow row;
    char *start = text;
    char *end = start + strlen(text);
    float h = 0;
    float w = 0;

    nvgFontFaceId(ctx, style->font.face);
    nvgFontSize(ctx, style->font.size);
    nvgTextAlign(ctx, style->font.align);

    while (nvgTextBreakLines(ctx, start, end, style->box.w, &row, 1))
    {

        if (w < row.width)
            w = row.width;

        start = (char *)row.next;
        h += style->font.height;

    }

    style->box.w = w;
    style->box.h = h;

}

static void alfi_style_fillrect(struct alfi_style *style)
{

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, style->box.x, style->box.y, style->box.w, style->box.h, style->radius);
    nvgFillColor(ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));
    nvgFill(ctx);

}

static void alfi_style_fillborder(struct alfi_style *style, float bordersize)
{

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, style->box.x, style->box.y, style->box.w, style->box.h, style->radius);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgRoundedRect(ctx, style->box.x + bordersize, style->box.y + bordersize, style->box.w - bordersize * 2, style->box.h - bordersize * 2, style->radius);
    nvgFillColor(ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));
    nvgFill(ctx);

}

static void alfi_style_paintrect(struct alfi_style *style, struct nvg_paint paint)
{

    nvgBeginPath(ctx);
    nvgRect(ctx, style->box.x, style->box.y, style->box.w, style->box.h);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);

}

static void alfi_style_fillcircle(struct alfi_style *style)
{

    nvgBeginPath(ctx);
    nvgCircle(ctx, style->box.x, style->box.y, style->radius);
    nvgFillColor(ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));
    nvgFill(ctx);

}

static void alfi_style_filltext(struct alfi_style *style, char *text)
{

    struct nvg_textrow row;
    char *start = text;
    char *end = start + strlen(text);
    float y = style->box.y;

    nvgFontFaceId(ctx, style->font.face);
    nvgFontSize(ctx, style->font.size);
    nvgTextAlign(ctx, style->font.align);

    while (nvgTextBreakLines(ctx, start, end, style->box.w, &row, 1))
    {

        nvgFillColor(ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));
        nvgText(ctx, style->box.x, y, row.start, row.end);

        start = (char *)row.next;
        y += style->font.height;

    }

}

static void alfi_style_filltext1(struct alfi_style *style, char *text)
{

    alfi_style_fittext(style, text);
    alfi_style_filltext(style, text);

}

static void alfi_style_filltext2(struct alfi_style *style, char *text)
{

    struct alfi_style white;

    alfi_style_fittext(style, text);
    box_clone(&white.box, &style->box);
    box_pad(&white.box, -marginx, -marginy);
    color_clone(&white.color, &color_background);
    alfi_style_fillrect(&white);
    alfi_style_filltext(style, text);

}

static void alfi_style_tween(struct alfi_style *s1, struct alfi_style *s2, float u)
{

    box_lerpfrom(&s1->box, &s2->box, u);
    color_lerpfrom(&s1->color, &s2->color, u);
    font_lerpfrom(&s1->font, &s2->font, u);

    s1->radius = flerp(s2->radius, s1->radius, u);

}

static void widget_anchor_keyframe(struct alfi_frame_anchor *keyframe, struct alfi_widget_anchor *anchor, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        font_init(&keyframe->label.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, marginx, marginy);
        alfi_style_fittext(&keyframe->label, anchor->label.content);
        color_clone(&keyframe->label.color, &color_focus);
        box_init(&keyframe->background.box, x, y, keyframe->label.box.w, keyframe->label.box.h);
        box_pad(&keyframe->background.box, -marginx, -marginy);
        color_clone(&keyframe->background.color, &color_background);

        break;

    }

}

static void widget_anchor_init(struct alfi_widget *widget)
{

    if (!widget->data.anchor.label.content)
    {

        widget->data.anchor.label.content = parser.createstring(1);

        memcpy(widget->data.anchor.label.content, "", 1);

    }

}

static void widget_anchor_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.anchor.label.content);

}

static void widget_anchor_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_anchor keyframe;

    widget_anchor_keyframe(&keyframe, &widget->data.anchor, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.anchor.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.anchor.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.anchor.frame.label, widget->data.anchor.label.content);
    box_clone(&widget->bb, &widget->data.anchor.frame.background.box);

}

static void widget_anchor_render(struct alfi_widget *widget)
{

    alfi_style_filltext1(&widget->data.anchor.frame.label, widget->data.anchor.label.content);

}

static unsigned int widget_anchor_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_anchor_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

    if (strlen(widget->data.anchor.link.url))
        createpage(widget->data.anchor.link.url);

}

static unsigned int widget_anchor_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_button_keyframe(struct alfi_frame_button *keyframe, struct alfi_widget_button *button, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_FOCUS:
        font_init(&keyframe->label.font, fontface_bold, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, gridx, gridy);
        alfi_style_fittext(&keyframe->label, button->label.content);
        color_init(&keyframe->label.color, 255, 255, 255, 255);
        box_init(&keyframe->background.box, x, y, w, h);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->border.box, x, y, w, h);
        box_pad(&keyframe->border.box, marginx, marginy);
        color_clone(&keyframe->border.color, &color_focus);

        keyframe->border.radius = 4.0;

        break;

    case ALFI_STATE_HOVER:
    case ALFI_STATE_NORMAL:
        font_init(&keyframe->label.font, fontface_bold, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, gridx, gridy);
        alfi_style_fittext(&keyframe->label, button->label.content);
        color_init(&keyframe->label.color, 255, 255, 255, 255);
        box_init(&keyframe->background.box, x, y, w, h);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->border.box, x, y, w, h);
        box_pad(&keyframe->border.box, marginx, marginy);
        color_clone(&keyframe->border.color, &color_focus);

        keyframe->border.radius = 4.0;

        break;

    }

}

static void widget_button_init(struct alfi_widget *widget)
{

    if (!widget->data.button.label.content)
    {

        widget->data.button.label.content = parser.createstring(10);

        memcpy(widget->data.button.label.content, "undefined", 10);

    }

}

static void widget_button_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.button.label.content);

}

static void widget_button_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_button keyframe;
    float selfh = gridy * 3;

    widget_button_keyframe(&keyframe, &widget->data.button, widget->state, x, y, w, selfh);
    alfi_style_tween(&widget->data.button.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.button.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.button.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.button.frame.label, widget->data.button.label.content);
    box_clone(&widget->bb, &widget->data.button.frame.background.box);

}

static void widget_button_render(struct alfi_widget *widget)
{

    alfi_style_fillrect(&widget->data.button.frame.border);

    if (strlen(widget->data.button.label.content))
        alfi_style_filltext1(&widget->data.button.frame.label, widget->data.button.label.content);

}

static unsigned int widget_button_setstate(struct alfi_widget *widget, unsigned int state)
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

static void widget_button_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_button_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_choice_keyframe(struct alfi_frame_choice *keyframe, struct alfi_widget_choice *choice, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_HOVER:
        font_init(&keyframe->label.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, marginx, marginy);
        alfi_style_fittext(&keyframe->label, choice->label.content);
        color_clone(&keyframe->label.color, &color_text);
        box_init(&keyframe->background.box, x, y, w, keyframe->label.box.h + marginy * 2);
        color_init(&keyframe->background.color, 224, 224, 224, 255);

        keyframe->background.radius = 4.0;

        break;

    case ALFI_STATE_NORMAL:
        font_init(&keyframe->label.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, marginx, marginy);
        alfi_style_fittext(&keyframe->label, choice->label.content);
        color_clone(&keyframe->label.color, &color_text);
        box_init(&keyframe->background.box, x, y, w, keyframe->label.box.h + marginy * 2);
        color_clone(&keyframe->background.color, &color_background);

        keyframe->background.radius = 4.0;

        break;

    }

}

static void widget_choice_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_choice keyframe;

    widget_choice_keyframe(&keyframe, &widget->data.choice, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.choice.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.choice.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.choice.frame.label, widget->data.choice.label.content);
    box_clone(&widget->bb, &widget->data.choice.frame.background.box);

}

static void widget_choice_init(struct alfi_widget *widget)
{

    if (!widget->data.choice.label.content)
    {

        widget->data.choice.label.content = parser.createstring(1);

        memcpy(widget->data.choice.label.content, "", 1);

    }

}

static void widget_choice_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.choice.label.content);

}

static void widget_choice_render(struct alfi_widget *widget)
{

    alfi_style_fillrect(&widget->data.choice.frame.background);

    if (strlen(widget->data.choice.label.content))
        alfi_style_filltext1(&widget->data.choice.frame.label, widget->data.choice.label.content);

}

static unsigned int widget_choice_setstate(struct alfi_widget *widget, unsigned int state)
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

static void widget_choice_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_choice_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_divider_keyframe(struct alfi_frame_divider *keyframe, struct alfi_widget_divider *divider, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        font_init(&keyframe->label.font, fontface_regular, fontsize_small, fontsize_small, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        alfi_style_fittext(&keyframe->label, divider->label.content);
        box_translate(&keyframe->label.box, w * 0.5 - keyframe->label.box.w * 0.5, gridy - keyframe->label.box.h * 0.5);
        color_clone(&keyframe->label.color, &color_text);
        box_init(&keyframe->background.box, x, y, w, gridy * 2);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->border.box, x + marginx, y + gridy - 2, w - marginx * 2, 2);
        color_clone(&keyframe->border.color, &color_line);

        break;

    }

}

static void widget_divider_init(struct alfi_widget *widget)
{

    if (!widget->data.divider.label.content)
    {

        widget->data.divider.label.content = parser.createstring(1);

        memcpy(widget->data.divider.label.content, "", 1);

    }

}

static void widget_divider_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.divider.label.content);

}

static void widget_divider_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_divider keyframe;

    widget_divider_keyframe(&keyframe, &widget->data.divider, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.divider.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.divider.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.divider.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.divider.frame.label, widget->data.divider.label.content);
    box_clone(&widget->bb, &widget->data.divider.frame.background.box);

}

static void widget_divider_render(struct alfi_widget *widget)
{

    alfi_style_fillrect(&widget->data.divider.frame.border);

    if (strlen(widget->data.divider.label.content))
        alfi_style_filltext2(&widget->data.divider.frame.label, widget->data.divider.label.content);

}

static unsigned int widget_divider_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_divider_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_divider_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_ARROW;

}

static void widget_field_keyframe(struct alfi_frame_field *keyframe, struct alfi_widget_field *field, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_FOCUS:
        font_init(&keyframe->label.font, fontface_regular, fontsize_small, fontsize_small, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        alfi_style_fittext(&keyframe->label, field->label.content);
        box_translate(&keyframe->label.box, gridx, 0);
        color_clone(&keyframe->label.color, &color_focus);
        font_init(&keyframe->data.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->data.box, x, y, w, h);
        box_pad(&keyframe->data.box, gridx + marginx, gridy + marginy);
        alfi_style_fittext(&keyframe->data, field->data.content);
        color_clone(&keyframe->data.color, &color_text);
        box_init(&keyframe->background.box, x, y, w, h);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->border.box, x, y, w, h);
        box_pad(&keyframe->border.box, marginx, marginy);
        color_clone(&keyframe->border.color, &color_focus);

        keyframe->border.radius = 4.0;

        break;

    case ALFI_STATE_HOVER:
    case ALFI_STATE_NORMAL:
        if (strlen(field->data.content))
        {

            font_init(&keyframe->label.font, fontface_regular, fontsize_small, fontsize_small, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            box_init(&keyframe->label.box, x, y, w, h);
            alfi_style_fittext(&keyframe->label, field->label.content);
            box_translate(&keyframe->label.box, gridx, 0);
            color_clone(&keyframe->label.color, &color_line);

        }

        else
        {

            font_init(&keyframe->label.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            box_init(&keyframe->label.box, x, y, w, h);
            box_pad(&keyframe->label.box, gridx, gridy);
            alfi_style_fittext(&keyframe->label, field->label.content);
            color_clone(&keyframe->label.color, &color_line);

        }

        font_init(&keyframe->data.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->data.box, x, y, w, h);
        box_pad(&keyframe->data.box, gridx, gridy);
        alfi_style_fittext(&keyframe->data, field->data.content);
        color_clone(&keyframe->data.color, &color_text);
        box_init(&keyframe->background.box, x, y, w, h);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->border.box, x, y, w, h);
        box_pad(&keyframe->border.box, marginx, marginy);
        color_clone(&keyframe->border.color, &color_line);

        keyframe->border.radius = 4.0;

        break;

    }

}

static void widget_field_init(struct alfi_widget *widget)
{

    if (!widget->data.field.label.content)
    {

        widget->data.field.label.content = parser.createstring(1);

        memcpy(widget->data.field.label.content, "", 1);

    }

    if (!widget->data.field.data.content)
    {

        widget->data.field.data.total = ALFI_DATASIZE;
        widget->data.field.data.content = parser.createstring(widget->data.field.data.total);
        widget->data.field.data.offset = 0;

        memcpy(widget->data.field.data.content, "", 1);

    }

}

static void widget_field_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.field.label.content);
    parser.destroystring(widget->data.field.data.content);

}

static void widget_field_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_field keyframe;
    float selfh = gridy * 3;

    if (widget->state == ALFI_STATE_FOCUS)
        selfh = gridy * 4;

    widget_field_keyframe(&keyframe, &widget->data.field, widget->state, x, y, w, selfh);
    alfi_style_tween(&widget->data.field.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.field.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.field.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.field.frame.label, widget->data.field.label.content);
    alfi_style_tween(&widget->data.field.frame.data, &keyframe.data, u);
    alfi_style_fittext(&widget->data.field.frame.data, widget->data.field.data.content);
    box_clone(&widget->bb, &widget->data.field.frame.background.box);

}

static void widget_field_render(struct alfi_widget *widget)
{

    alfi_style_fillborder(&widget->data.field.frame.border, 2.0);

    if (strlen(widget->data.field.label.content))
        alfi_style_filltext2(&widget->data.field.frame.label, widget->data.field.label.content);

    if (strlen(widget->data.field.data.content))
        alfi_style_filltext2(&widget->data.field.frame.data, widget->data.field.data.content);

}

static unsigned int widget_field_setstate(struct alfi_widget *widget, unsigned int state)
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

static void widget_field_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_field_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_IBEAM;

}

static void widget_header_keyframe(struct alfi_frame_header *keyframe, struct alfi_widget_header *header, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        font_init(&keyframe->label.font, fontface_bold, fontsize_xlarge, fontsize_xlarge, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, marginx, marginy);
        alfi_style_fittext(&keyframe->label, header->label.content);
        color_clone(&keyframe->label.color, &color_header);
        box_init(&keyframe->background.box, x, y, keyframe->label.box.w, keyframe->label.box.h);
        box_pad(&keyframe->background.box, -marginx, -marginy);
        box_translate(&keyframe->background.box, marginx, marginy);
        color_clone(&keyframe->background.color, &color_background);

        break;

    }

}

static void widget_header_init(struct alfi_widget *widget)
{

    if (!widget->data.header.label.content)
    {

        widget->data.header.label.content = parser.createstring(1);

        memcpy(widget->data.header.label.content, "", 1);

    }

}

static void widget_header_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.header.label.content);

}

static void widget_header_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_header keyframe;

    widget_header_keyframe(&keyframe, &widget->data.header, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.header.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.header.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.header.frame.label, widget->data.header.label.content);
    box_clone(&widget->bb, &widget->data.header.frame.background.box);

}

static void widget_header_render(struct alfi_widget *widget)
{

    if (strlen(widget->data.header.label.content))
        alfi_style_filltext1(&widget->data.header.frame.label, widget->data.header.label.content);

}

static unsigned int widget_header_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_header_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_header_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_IBEAM;

}

static void widget_hstack_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_widget *child = 0;
    float selfw = w;
    float selfh = h;
    float cx = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y, 0, h, u);

        cx += child->bb.w;

        if (selfw < cx)
            selfw = cx;

        if (selfh < child->bb.h)
            selfh = child->bb.h;

    }

    box_init(&widget->bb, x, y, selfw, selfh);

}

static void widget_hstack_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        call_render(child);

}

static unsigned int widget_hstack_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_hstack_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_hstack_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_ARROW;

}

static void widget_image_keyframe(struct alfi_frame_image *keyframe, struct alfi_widget_image *image, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        box_init(&keyframe->frame.box, x, y, image->resource.w, image->resource.h);
        box_translate(&keyframe->frame.box, marginx, marginy);
        box_init(&keyframe->background.box, keyframe->frame.box.x, keyframe->frame.box.y, keyframe->frame.box.w, keyframe->frame.box.h);
        box_pad(&keyframe->background.box, -marginx, -marginy);
        color_clone(&keyframe->background.color, &color_background);

        break;

    }

}

static void widget_image_init(struct alfi_widget *widget)
{

    widget->data.image.resource.ref = nvgCreateImage(ctx, widget->data.image.link.url, 0);

    nvgImageSize(ctx, widget->data.image.resource.ref, &widget->data.image.resource.w, &widget->data.image.resource.h);

}

static void widget_image_destroy(struct alfi_widget *widget)
{

}

static void widget_image_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_image keyframe;

    widget_image_keyframe(&keyframe, &widget->data.image, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.image.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.image.frame.frame, &keyframe.frame, u);
    box_clone(&widget->bb, &widget->data.image.frame.background.box);

}

static void widget_image_render(struct alfi_widget *widget)
{

    struct nvg_paint paint = nvgImagePattern(ctx, widget->data.image.frame.frame.box.x, widget->data.image.frame.frame.box.y, widget->data.image.frame.frame.box.w, widget->data.image.frame.frame.box.h, 0.0, widget->data.image.resource.ref, 1.0);

    alfi_style_paintrect(&widget->data.image.frame.frame, paint);

}

static unsigned int widget_image_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_image_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_image_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_ARROW;

}

static void widget_list_keyframe(struct alfi_frame_list *keyframe, struct alfi_widget_list *list, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        keyframe->dot.radius = 3.0;

        color_clone(&keyframe->dot.color, &color_header);

        break;

    }

}

static void widget_list_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_list keyframe;
    struct alfi_widget *child = 0;
    float padx = gridx * 2;
    float selfw = w;
    float selfh = h;
    float cy = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + padx, y + cy, w - padx, 0, u);

        cy += child->bb.h;

        if (selfw < child->bb.w)
            selfw = child->bb.w;

        if (selfh < cy)
            selfh = cy;

    }

    widget_list_keyframe(&keyframe, &widget->data.list, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->data.list.frame.dot, &keyframe.dot, u);
    box_init(&widget->bb, x, y, selfw, selfh);

}

static void widget_list_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        call_render(child);

        if (!call_checkflag(widget, ALFI_FLAG_ITEM))
            continue;

        widget->data.list.frame.dot.box.x = child->bb.x - marginx;
        widget->data.list.frame.dot.box.y = child->bb.y + child->bb.h * 0.5;

        alfi_style_fillcircle(&widget->data.list.frame.dot);

    }

}

static unsigned int widget_list_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_list_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_list_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_ARROW;

}

static void widget_select_keyframe(struct alfi_frame_select *keyframe, struct alfi_widget_select *select, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_FOCUS:
        font_init(&keyframe->label.font, fontface_regular, fontsize_small, fontsize_small, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_translate(&keyframe->label.box, gridx, 0);
        alfi_style_fittext(&keyframe->label, select->label.content);
        color_clone(&keyframe->label.color, &color_focus);
        font_init(&keyframe->data.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->data.box, x, y, w, h);
        box_pad(&keyframe->data.box, gridx + marginx, gridy + marginy);
        alfi_style_fittext(&keyframe->data, select->data.content);
        color_clone(&keyframe->data.color, &color_text);
        box_init(&keyframe->background.box, x, y, w, h);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->border.box, x, y, w, h);
        box_pad(&keyframe->border.box, marginx, marginy);
        color_clone(&keyframe->border.color, &color_focus);

        keyframe->border.radius = 4.0;

        break;

    case ALFI_STATE_HOVER:
    case ALFI_STATE_NORMAL:
        if (strlen(select->data.content))
        {

            font_init(&keyframe->label.font, fontface_regular, fontsize_small, fontsize_small, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            box_init(&keyframe->label.box, x, y, w, h);
            box_translate(&keyframe->label.box, gridx, 0);
            alfi_style_fittext(&keyframe->label, select->label.content);
            color_clone(&keyframe->label.color, &color_line);

        }

        else
        {

            font_init(&keyframe->label.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            box_init(&keyframe->label.box, x, y, w, h);
            box_translate(&keyframe->label.box, gridx, gridy);
            alfi_style_fittext(&keyframe->label, select->label.content);
            color_clone(&keyframe->label.color, &color_line);

        }

        font_init(&keyframe->data.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->data.box, x, y, w, h);
        box_pad(&keyframe->data.box, gridx, gridy);
        alfi_style_fittext(&keyframe->data, select->data.content);
        color_clone(&keyframe->data.color, &color_text);
        box_init(&keyframe->background.box, x, y, w, h);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->border.box, x, y, w, h);
        box_pad(&keyframe->border.box, marginx, marginy);
        color_clone(&keyframe->border.color, &color_line);

        keyframe->border.radius = 4.0;

        break;

    }

}

static void widget_select_init(struct alfi_widget *widget)
{

    if (!widget->data.select.label.content)
    {

        widget->data.select.label.content = parser.createstring(1);

        memcpy(widget->data.select.label.content, "", 1);

    }

    if (!widget->data.select.data.content)
    {

        widget->data.select.data.total = ALFI_DATASIZE;
        widget->data.select.data.content = parser.createstring(widget->data.select.data.total);
        widget->data.select.data.offset = 0;

        memcpy(widget->data.select.data.content, "", 1);

    }

}

static void widget_select_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.select.label.content);
    parser.destroystring(widget->data.select.data.content);

}

static void widget_select_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_select keyframe;
    struct alfi_widget *child = 0;
    float selfw = w;
    float selfh = gridy * 3;
    float cy = gridy * 3;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + gridx, y + cy, w - gridx * 2, 0, u);

        cy += child->bb.h;

        if (widget->state == ALFI_STATE_FOCUS)
        {

            if (selfw < child->bb.w)
                selfw = child->bb.w;

            if (selfh < cy + gridy)
                selfh = cy + gridy;

        }

    }

    widget_select_keyframe(&keyframe, &widget->data.select, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->data.select.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.select.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.select.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.select.frame.label, widget->data.select.label.content);
    alfi_style_tween(&widget->data.select.frame.data, &keyframe.data, u);
    alfi_style_fittext(&widget->data.select.frame.data, widget->data.select.data.content);
    box_clone(&widget->bb, &widget->data.select.frame.background.box);

}

static void widget_select_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    alfi_style_fillborder(&widget->data.select.frame.border, 2.0);

    if (strlen(widget->data.select.label.content))
        alfi_style_filltext2(&widget->data.select.frame.label, widget->data.select.label.content);

    if (strlen(widget->data.select.data.content))
        alfi_style_filltext2(&widget->data.select.frame.data, widget->data.select.data.content);

    if (widget->state == ALFI_STATE_FOCUS)
    {

        while ((child = pool_nextchild(child, widget)))
            call_render(child);

    }

}

static unsigned int widget_select_setstate(struct alfi_widget *widget, unsigned int state)
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

static void widget_select_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_select_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_subheader_keyframe(struct alfi_frame_subheader *keyframe, struct alfi_widget_subheader *subheader, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        font_init(&keyframe->label.font, fontface_bold, fontsize_large, fontsize_large, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, marginx, marginy);
        alfi_style_fittext(&keyframe->label, subheader->label.content);
        color_clone(&keyframe->label.color, &color_header);
        box_init(&keyframe->background.box, x, y, keyframe->label.box.w, keyframe->label.box.h);
        box_pad(&keyframe->background.box, -marginx, -marginy);
        box_translate(&keyframe->background.box, marginx, marginy);
        color_clone(&keyframe->background.color, &color_background);

        break;

    }

}

static void widget_subheader_init(struct alfi_widget *widget)
{

    if (!widget->data.subheader.label.content)
    {

        widget->data.subheader.label.content = parser.createstring(1);

        memcpy(widget->data.subheader.label.content, "", 1);

    }

}

static void widget_subheader_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.subheader.label.content);

}

static void widget_subheader_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_subheader keyframe;

    widget_subheader_keyframe(&keyframe, &widget->data.subheader, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.subheader.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.subheader.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.subheader.frame.label, widget->data.subheader.label.content);
    box_clone(&widget->bb, &widget->data.subheader.frame.background.box);

}

static void widget_subheader_render(struct alfi_widget *widget)
{

    if (strlen(widget->data.subheader.label.content))
        alfi_style_filltext1(&widget->data.subheader.frame.label, widget->data.subheader.label.content);

}

static unsigned int widget_subheader_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_subheader_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_subheader_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_IBEAM;

}

static void widget_table_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_widget *child = 0;
    float childw = gridx * widget->data.table.grid.clength;
    float childh = gridy * widget->data.table.grid.rlength;
    float cx = 0;
    float cy = 0;
    float selfw = 0;
    float selfh = 0;
    float rowh = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y + cy, childw, childh, u);

        cx += childw;

        if (rowh < child->bb.h)
            rowh = child->bb.h;

        if (selfw < cx)
            selfw = cx;

        if (cx >= gridx * widget->data.table.grid.csize)
        {

            cx = 0;

            if (widget->data.table.grid.rlength)
                cy += childh;
            else
                cy += rowh;

            selfh += rowh;
            rowh = 0;

        }

    }

    box_init(&widget->bb, x, y, selfw, selfh + rowh);

}

static void widget_table_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        call_render(child);

}

static unsigned int widget_table_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_table_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_table_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_ARROW;

}

static void widget_text_keyframe(struct alfi_frame_text *keyframe, struct alfi_widget_text *text, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        font_init(&keyframe->label.font, fontface_regular, fontsize_medium, fontsize_medium, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h);
        box_pad(&keyframe->label.box, marginx, marginy);
        alfi_style_fittext(&keyframe->label, text->label.content);
        color_clone(&keyframe->label.color, &color_text);
        box_init(&keyframe->background.box, x, y, keyframe->label.box.w, keyframe->label.box.h);
        box_pad(&keyframe->background.box, -marginx, -marginy);
        box_translate(&keyframe->background.box, marginx, marginy);
        color_clone(&keyframe->background.color, &color_background);

        break;

    }

}

static void widget_text_init(struct alfi_widget *widget)
{

    if (!widget->data.text.label.content)
    {

        widget->data.text.label.content = parser.createstring(1);

        memcpy(widget->data.text.label.content, "", 1);

    }

}

static void widget_text_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.text.label.content);

}

static void widget_text_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_text keyframe;

    widget_text_keyframe(&keyframe, &widget->data.text, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.text.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.text.frame.label, &keyframe.label, u);
    alfi_style_fittext(&widget->data.text.frame.label, widget->data.text.label.content);
    box_clone(&widget->bb, &widget->data.text.frame.background.box);

}

static void widget_text_render(struct alfi_widget *widget)
{

    if (strlen(widget->data.text.label.content))
        alfi_style_filltext1(&widget->data.text.frame.label, widget->data.text.label.content);

}

static unsigned int widget_text_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_text_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_text_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_IBEAM;

}

static void widget_vstack_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_widget *child = 0;
    float selfw = w;
    float selfh = h;
    float cy = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x, y + cy, w, 0, u);

        cy += child->bb.h;

        if (selfw < child->bb.w)
            selfw = child->bb.w;

        if (selfh < cy)
            selfh = cy;

    }

    box_init(&widget->bb, x, y, selfw, selfh);

}

static void widget_vstack_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        call_render(child);

}

static unsigned int widget_vstack_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_vstack_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_vstack_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_ARROW;

}

static void widget_window_keyframe(struct alfi_frame_window *keyframe, struct alfi_widget_window *window, unsigned int state, float x, float y, float w, float h)
{

    switch (state)
    {

    case ALFI_STATE_NORMAL:
        box_init(&keyframe->background.box, x, y, w, h);
        color_clone(&keyframe->background.color, &color_background);
        box_init(&keyframe->shadow1.box, x, y, w, h);
        color_init(&keyframe->shadow1.color, 0, 0, 0, 48);
        box_init(&keyframe->shadow2.box, x - marginx - 16.0, y - marginy - 16.0, w + marginx * 2 + 16.0 * 2, h + marginy * 2 + 16.0 * 2);
        color_init(&keyframe->shadow2.color, 0, 0, 0, 0);

        keyframe->background.radius = 32.0;
        keyframe->shadow1.radius = 32.0;
        keyframe->shadow2.radius = 32.0;

        break;

    }

}

static void widget_window_init(struct alfi_widget *widget)
{

    if (!widget->data.window.label.content)
    {

        widget->data.window.label.content = parser.createstring(10);

        memcpy(widget->data.window.label.content, "undefined", 10);

    }

}

static void widget_window_destroy(struct alfi_widget *widget)
{

    parser.destroystring(widget->data.window.label.content);

}

static void widget_window_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_window keyframe;
    struct alfi_widget *child = 0;
    float selfw = gridx * 28;
    float selfh = gridy * 28;
    float padx = gridx * 2;
    float pady = gridy * 2;
    float childw = selfw - padx * 2;
    float childh = selfh - pady * 2;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + padx, y + pady, childw, childh, u);

        if (selfw < child->bb.w + padx * 2)
            selfw = child->bb.w + padx * 2;

        if (selfh < child->bb.h + pady * 2)
            selfh = child->bb.h + pady * 2;

    }

    widget_window_keyframe(&keyframe, &widget->data.window, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->data.window.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.window.frame.shadow1, &keyframe.shadow1, u);
    alfi_style_tween(&widget->data.window.frame.shadow2, &keyframe.shadow2, u);
    box_init(&widget->bb, x, y, selfw, selfh);

}

static void widget_window_render(struct alfi_widget *widget)
{

    struct nvg_paint paint = nvgBoxGradient(ctx, widget->data.window.frame.shadow1.box.x, widget->data.window.frame.shadow1.box.y, widget->data.window.frame.shadow1.box.w, widget->data.window.frame.shadow1.box.h, widget->data.window.frame.shadow1.radius, 16.0, nvgRGBA(widget->data.window.frame.shadow1.color.r, widget->data.window.frame.shadow1.color.g, widget->data.window.frame.shadow1.color.b, widget->data.window.frame.shadow1.color.a), nvgRGBA(widget->data.window.frame.shadow2.color.r, widget->data.window.frame.shadow2.color.g, widget->data.window.frame.shadow2.color.b, widget->data.window.frame.shadow2.color.a));
    struct alfi_widget *child = 0;

    alfi_style_paintrect(&widget->data.window.frame.shadow2, paint);
    alfi_style_fillrect(&widget->data.window.frame.background);

    while ((child = pool_nextchild(child, widget)))
        call_render(child);

}

static unsigned int widget_window_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_window_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

}

static unsigned int widget_window_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_ARROW;

}

static void setsize(float x, float y)
{

    gridx = x / 40;
    gridy = y / 40;

    if (gridx < 24)
        gridx = 24;

    if (gridx > 32)
        gridx = 32;

    if (gridy < 24)
        gridy = 24;

    if (gridy > 32)
        gridy = 32;

    fontsize_small = gridy / 2 + gridy / 4;
    fontsize_medium = gridy;
    fontsize_large = gridy + gridy / 3;
    fontsize_xlarge = gridy + gridy;

}

static void setdefaulttheme(void)
{

    color_init(&color_fade1, 160, 160, 160, 255);
    color_init(&color_fade2, 192, 192, 192, 255);
    color_init(&color_background, 255, 255, 255, 255);
    color_init(&color_text, 96, 96, 96, 255);
    color_init(&color_header, 64, 64, 64, 255);
    color_init(&color_focus, 96, 192, 192, 255);
    color_init(&color_line, 192, 192, 192, 255);

}

static void setdarktheme(void)
{

    color_init(&color_fade1, 8, 8, 8, 255);
    color_init(&color_fade2, 16, 16, 16, 255);
    color_init(&color_background, 24, 24, 24, 255);
    color_init(&color_text, 128, 128, 128, 255);
    color_init(&color_header, 224, 224, 224, 255);
    color_init(&color_focus, 32, 128, 128, 255);
    color_init(&color_line, 48, 48, 48, 255);

}

static void setcursor(struct GLFWwindow *window, unsigned int type)
{

    switch (type)
    {

    case ALFI_CURSOR_HAND:
        glfwSetCursor(window, cursor_hand);

        break;

    case ALFI_CURSOR_IBEAM:
        glfwSetCursor(window, cursor_ibeam);

        break;

    case ALFI_CURSOR_ARROW:
        glfwSetCursor(window, cursor_arrow);

        break;

    default:
        glfwSetCursor(window, cursor_arrow);

        break;

    }

}

static void checktouch(GLFWwindow *window)
{

    struct alfi_widget *widget = findwidget(mx, my);

    sethover(widget);

    if (widget)
        setcursor(window, call_getcursor(widget, mx, my));

}

static void render(unsigned int group)
{

    struct nvg_paint paint = nvgLinearGradient(ctx, 0, 0, 0, winh, nvgRGBA(color_fade1.r, color_fade1.g, color_fade1.b, color_fade1.a), nvgRGBA(color_fade2.r, color_fade2.g, color_fade2.b, color_fade2.a));
    struct alfi_widget *widget = 0;
    struct alfi_style background;

    box_init(&background.box, 0, 0, winw, winh);
    alfi_style_paintrect(&background, paint);

    while ((widget = pool_nextingroupoftype(widget, group, ALFI_WIDGET_WINDOW)))
        call_render(widget);

}

static void onerror(int error, const char *desc)
{

    printf("GLFW error %d: %s\n", error, desc);

}

static void onwindowsize(GLFWwindow *window, int width, int height)
{

    winw = width;
    winh = height;

    setsize(winw, winh);

}

static void onframebuffersize(GLFWwindow *window, int width, int height)
{

    fbw = width;
    fbh = height;

    glViewport(0, 0, fbw, fbh);

}

static void onkey(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS)
    {

        switch (key)
        {

        case GLFW_KEY_D:
            if (mods & GLFW_MOD_CONTROL)
                createpage(url_default);

            break;

        case GLFW_KEY_H:
            if (mods & GLFW_MOD_CONTROL)
                createpage(url_home);

            break;

        case GLFW_KEY_M:
            if (mods & GLFW_MOD_CONTROL)
                setdarktheme();

            break;

        case GLFW_KEY_N:
            if (mods & GLFW_MOD_CONTROL)
                setdefaulttheme();

            break;

        case GLFW_KEY_Q:
            if (mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(window, GL_TRUE);

            break;

        case GLFW_KEY_T:
            if (mods & GLFW_MOD_CONTROL)
                createpage(url_test);

            break;

        }

    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch (key)
        {

        case GLFW_KEY_UP:
            scrolly += gridy;

            break;

        case GLFW_KEY_DOWN:
            scrolly -= gridy;

            break;

        case GLFW_KEY_LEFT:
            scrollx += gridx;

            break;

        case GLFW_KEY_RIGHT:
            scrollx -= gridx;

            break;

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

        case GLFW_KEY_TAB:
            {

                struct alfi_widget *widget = (mods & GLFW_MOD_SHIFT) ? pool_prevflag(focus, ALFI_FLAG_FOCUSABLE) : pool_nextflag(focus, ALFI_FLAG_FOCUSABLE);

                if (!widget)
                    widget = (mods & GLFW_MOD_SHIFT) ? pool_prevflag(0, ALFI_FLAG_FOCUSABLE) : pool_nextflag(0, ALFI_FLAG_FOCUSABLE);

                setfocus(widget);

            }

            break;

        case GLFW_KEY_ENTER:
            if (focus)
            {

                if (focus->type == ALFI_WIDGET_FIELD)
                {

                    if (focus->data.field.data.offset < focus->data.field.data.total - 1)
                    {

                        focus->data.field.data.content[focus->data.field.data.offset] = '\n';
                        focus->data.field.data.offset++;
                        focus->data.field.data.content[focus->data.field.data.offset] = '\0';

                    }

                }

            }

            break;

        case GLFW_KEY_BACKSPACE:
            if (focus)
            {

                if (focus->type == ALFI_WIDGET_FIELD)
                {

                    if (focus->data.field.data.offset > 0)
                    {

                        focus->data.field.data.offset--;
                        focus->data.field.data.content[focus->data.field.data.offset] = '\0';

                    }

                }

            }

            break;

        }

    }

}

static void onbutton(GLFWwindow *window, int button, int action, int mods)
{

    if (action == GLFW_PRESS)
    {

        switch (button)
        {

        case GLFW_MOUSE_BUTTON_LEFT:
            if (hover)
                call_onclick(hover);

            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (hover)
            {

                parser_destroy(hover);
                sethover(0);

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

static void onchar(GLFWwindow *window, unsigned int codepoint)
{

    if (focus)
    {

        if (focus->type == ALFI_WIDGET_FIELD)
        {

            if (focus->data.field.data.offset < focus->data.field.data.total - 1)
            {

                focus->data.field.data.content[focus->data.field.data.offset] = codepoint;
                focus->data.field.data.offset++;
                focus->data.field.data.content[focus->data.field.data.offset] = '\0';

            }

        }

    }

}

static void setupscroll(void)
{

    scrollx = gridx;
    scrolly = gridy;

}

static void setupfonts(void)
{

    fontface_regular = nvgCreateFont(ctx, "regular", "/usr/share/navi/roboto-regular.ttf");

    if (fontface_regular == -1)
    {

        printf("Could not add font.\n");

        return;

    }

    fontface_bold = nvgCreateFont(ctx, "bold", "/usr/share/navi/roboto-bold.ttf");

    if (fontface_bold == -1)
    {

        printf("Could not add font.\n");

        return;

    }

    fontface_icon = nvgCreateFont(ctx, "icon", "/usr/share/navi/icofont.ttf");

    if (fontface_icon == -1)
    {

        printf("Could not add font.\n");

        return;

    }

}

static void setupcursors(void)
{

    cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursor_ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursor_hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

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

    glfwSetWindowSizeCallback(window, onwindowsize);
    glfwSetFramebufferSizeCallback(window, onframebuffersize);
    glfwSetKeyCallback(window, onkey);
    glfwSetMouseButtonCallback(window, onbutton);
    glfwSetCursorPosCallback(window, oncursor);
    glfwSetScrollCallback(window, onscroll);
    glfwSetCharCallback(window, onchar);
    glfwMakeContextCurrent(window);
    glfwGetWindowSize(window, &winw, &winh);
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glfwSwapInterval(1);
    glfwSetTime(0);
    glewInit();

    ctx = nvg_create(0);

    if (ctx == 0)
    {

        printf("Could not init nanovg.\n");

        return -1;

    }

    parser_init(&parser, parser_fail, parser_find, parser_create, parser_destroy, parser_createstring, parser_destroystring);
    call_register(ALFI_WIDGET_ANCHOR, ALFI_FLAG_ITEM, widget_anchor_init, widget_anchor_destroy, widget_anchor_place, widget_anchor_render, widget_anchor_setstate, widget_anchor_onclick, widget_anchor_getcursor);
    call_register(ALFI_WIDGET_BUTTON, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_button_init, widget_button_destroy, widget_button_place, widget_button_render, widget_button_setstate, widget_button_onclick, widget_button_getcursor);
    call_register(ALFI_WIDGET_CHOICE, ALFI_FLAG_CONTAINER, widget_choice_init, widget_choice_destroy, widget_choice_place, widget_choice_render, widget_choice_setstate, widget_choice_onclick, widget_choice_getcursor);
    call_register(ALFI_WIDGET_DIVIDER, ALFI_FLAG_ITEM, widget_divider_init, widget_divider_destroy, widget_divider_place, widget_divider_render, widget_divider_setstate, widget_divider_onclick, widget_divider_getcursor);
    call_register(ALFI_WIDGET_HSTACK, ALFI_FLAG_CONTAINER, 0, 0, widget_hstack_place, widget_hstack_render, widget_hstack_setstate, widget_hstack_onclick, widget_hstack_getcursor);
    call_register(ALFI_WIDGET_FIELD, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_field_init, widget_field_destroy, widget_field_place, widget_field_render, widget_field_setstate, widget_field_onclick, widget_field_getcursor);
    call_register(ALFI_WIDGET_HEADER, ALFI_FLAG_ITEM, widget_header_init, widget_header_destroy, widget_header_place, widget_header_render, widget_header_setstate, widget_header_onclick, widget_header_getcursor);
    call_register(ALFI_WIDGET_IMAGE, ALFI_FLAG_ITEM, widget_image_init, widget_image_destroy, widget_image_place, widget_image_render, widget_image_setstate, widget_image_onclick, widget_image_getcursor);
    call_register(ALFI_WIDGET_LIST, ALFI_FLAG_CONTAINER, 0, 0, widget_list_place, widget_list_render, widget_list_setstate, widget_list_onclick, widget_list_getcursor);
    call_register(ALFI_WIDGET_SELECT, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_select_init, widget_select_destroy, widget_select_place, widget_select_render, widget_select_setstate, widget_select_onclick, widget_select_getcursor);
    call_register(ALFI_WIDGET_SUBHEADER, ALFI_FLAG_ITEM, widget_subheader_init, widget_subheader_destroy, widget_subheader_place, widget_subheader_render, widget_subheader_setstate, widget_subheader_onclick, widget_subheader_getcursor);
    call_register(ALFI_WIDGET_TABLE, ALFI_FLAG_CONTAINER, 0, 0, widget_table_place, widget_table_render, widget_table_setstate, widget_table_onclick, widget_table_getcursor);
    call_register(ALFI_WIDGET_TEXT, ALFI_FLAG_ITEM, widget_text_init, widget_text_destroy, widget_text_place, widget_text_render, widget_text_setstate, widget_text_onclick, widget_text_getcursor);
    call_register(ALFI_WIDGET_VSTACK, ALFI_FLAG_CONTAINER, 0, 0, widget_vstack_place, widget_vstack_render, widget_vstack_setstate, widget_vstack_onclick, widget_vstack_getcursor);
    call_register(ALFI_WIDGET_WINDOW, ALFI_FLAG_CONTAINER, widget_window_init, widget_window_destroy, widget_window_place, widget_window_render, widget_window_setstate, widget_window_onclick, widget_window_getcursor);

    pool_init();
    setupscroll();
    setupcursors();
    setupfonts();
    setsize(winw, winh);
    setdefaulttheme();
    createpage(url_default);

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
            place(0, 0.5);
            render(0);
            checktouch(window);
            nvgEndFrame(ctx);
            glfwSwapBuffers(window);

        }

        glfwPollEvents();

    }

    nvg_delete(ctx);
    glfwTerminate();

    return 0;

}

