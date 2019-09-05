#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "fons.h"
#include "nvg.h"
#include "nvg_gl.h"
#include "list.h"
#include "style.h"
#include "resource.h"
#include "widgets.h"
#include "parser.h"
#include "call.h"
#include "pool.h"

static struct parser parser;
static float margin_w;
static float margin_h;
static float unit_w;
static float unit_h;
static int view_x;
static int view_y;
static int view_w;
static int view_h;
static double mouse_x;
static double mouse_y;
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
static struct alfi_widget *hover;
static struct alfi_widget *focus;
static struct alfi_color color_background;
static struct alfi_color color_text;
static struct alfi_color color_header;
static struct alfi_color color_focus;
static struct alfi_color color_focustext;
static struct alfi_color color_line;
static struct nvg_context ctx;
static struct nvg_gl_context glctx;
static struct fons_context fsctx;
static struct resource res_page_default;
static struct resource res_page_home;
static struct resource res_page_test;
static struct resource res_page_current;
static struct resource res_font_regular;
static struct resource res_font_bold;
static struct resource res_font_icon;
static unsigned int updatetitle;

static struct alfi_widget *parser_findwidget(char *name)
{

    return pool_findbyname(name);

}

static char *parser_allocate(char *string, unsigned int size, unsigned int count, char *content)
{

    if (string)
    {

        free(string);

        string = 0;

    }

    if (size)
    {

        string = malloc(size);

        if (count)
            memcpy(string, content, count);

    }

    return string;

}

static char *parser_createstring(char *string, char *content)
{

    return parser.allocate(string, strlen(content) + 1, strlen(content) + 1, content);

}

static char *parser_destroystring(char *string)
{

    return parser.allocate(string, 0, 0, 0);

}

static struct alfi_widget *parser_createwidget(unsigned int type, char *in)
{

    struct alfi_widget *widget = pool_create();

    memset(&widget->data, 0, sizeof (union alfi_payload));

    widget->type = type;
    widget->id.name = parser.createstring(widget->id.name, "");
    widget->in.name = parser.createstring(widget->in.name, in);

    box_init(&widget->bb, 0, 0, 0, 0, 0);
    call_create(widget);
    call_setstate(widget, ALFI_STATE_NORMAL);

    return widget;

}

static struct alfi_widget *parser_destroywidget(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        child = parser_destroywidget(child);

    widget->id.name = parser.destroystring(widget->id.name);
    widget->in.name = parser.destroystring(widget->in.name);

    call_destroy(widget);
    pool_destroy(widget);

    return 0;

}

static void parser_fail(unsigned int line)
{

    printf("Parsing failed on line %u\n", line);
    exit(EXIT_FAILURE);

}

static void view_setsize(int w, int h)
{

    view_w = w;
    view_h = h;
    unit_w = view_w / 32;
    unit_h = view_h / 32;
    margin_w = 12;
    margin_h = 12;

    if (unit_w < 32)
        unit_w = 32;

    if (unit_h < 32)
        unit_h = 32;

}

static void view_setposition(float x, float y)
{

    view_x = x;
    view_y = y;

}

static void view_adjust(void)
{

    struct alfi_widget *window = pool_findbyname("window");

    if (!window)
        return;

    if (view_w < window->bb.w)
    {

        if (view_x > 0)
            view_x = 0;

        if (view_x < view_w - window->bb.w)
            view_x = view_w - window->bb.w;

    }

    else
    {

        view_x = view_w / 2 - window->bb.w / 2;

    }

    if (view_h < window->bb.h)
    {

        if (view_y > 0)
            view_y = 0;

        if (view_y < view_h - window->bb.h)
            view_y = view_h - window->bb.h;

    }

    else
    {

        view_y = 0;

    }

}

static void setcolors(unsigned int id)
{

    switch (id)
    {

    case 0:
        color_init(&color_background, 255, 255, 255, 255);
        color_init(&color_text, 96, 96, 96, 255);
        color_init(&color_header, 64, 64, 64, 255);
        color_init(&color_focus, 96, 192, 192, 255);
        color_init(&color_focustext, 255, 255, 255, 255);
        color_init(&color_line, 192, 192, 192, 255);

        break;

    case 1:
        color_init(&color_background, 24, 24, 24, 255);
        color_init(&color_text, 192, 192, 192, 255);
        color_init(&color_header, 224, 224, 224, 255);
        color_init(&color_focus, 96, 192, 192, 255);
        color_init(&color_focustext, 255, 255, 255, 255);
        color_init(&color_line, 96, 96, 96, 255);

        break;

    }

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

static void loadbase(void)
{

    struct alfi_widget *widget;

    widget = parser_createwidget(ALFI_WIDGET_WINDOW, "");

    if (widget)
    {

        widget->id.name = parser.createstring(widget->id.name, "window");
        widget->data.window.label.content = parser.createstring(widget->data.window.label.content, "undefined");

    }

    widget = parser_createwidget(ALFI_WIDGET_VSTACK, "window");

    if (widget)
    {

        widget->id.name = parser.createstring(widget->id.name, "main");
        widget->data.vstack.halign.direction = ALFI_HALIGN_LEFT;
        widget->data.vstack.valign.direction = ALFI_VALIGN_TOP;

    }

}

static void loadalfi(struct resource *resource)
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
            parser_parse(&parser, "main", count, data);

        close(fd[0]);

    }

    else
    {

        dup2(fd[0], 0);
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execlp("navi-resolve", "navi-resolve", resource->url, NULL);
        exit(EXIT_FAILURE);

    }

}

/*
static void loadimage(struct alfi_resource *resource, char *url)
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

        unsigned char *data = malloc(0x20000);
        int w, h, n;
        int ndata;
        unsigned char *img;

        close(fd[1]);

        ndata = read(fd[0], data, 0x20000);

        img = stbi_load_from_memory(data, ndata, &w, &h, &n, 4);

        if (img)
        {

            resource->ref = nvg_gl_createimagergba(&glctx, &ctx, w, h, 0, img);

            stbi_image_free(img);

        }

        close(fd[0]);

    }

    else
    {

        dup2(fd[0], 0);
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execlp("navi-resolve", "navi-resolve", url, NULL);
        exit(EXIT_FAILURE);

    }

}
*/

static void place(float x, float y, float u)
{

    struct alfi_widget *window = pool_findbyname("window");

    if (window)
        call_place(window, x, y, 0, 0, u);

}

static void clear(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        child = parser_destroywidget(child);

}

static void loadpart(char *url)
{

    struct resource res;

    resource_seturl(&res, url);
    loadalfi(&res);
    place(view_x, view_y, 1.0);

    updatetitle = 1;

}

static void loadmain(char *url)
{

    resource_seturl(&res_page_current, url);
    clear(pool_findbyname("main"));
    loadpart(res_page_current.url);
    view_setposition(0, 0);
    view_adjust();

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

static const char *calcline(struct alfi_style *style, const char *string, const char *end, float width, struct nvg_textrow *row)
{

    struct fons_textiter iter;
    struct fons_quad q;
    float rowStartX = 0;
    float rowWidth = 0;
    float rowMinX = 0;
    float rowMaxX = 0;
    const char *rowStart = NULL;
    const char *rowEnd = NULL;
    const char *wordStart = NULL;
    const char *breakEnd = NULL;
    float breakWidth = 0;
    float breakMaxX = 0;
    int type = NVG_SPACE;
    int ptype = NVG_SPACE;
    unsigned int pcodepoint = 0;

    fsctx.state.font = style->font.face;
    fsctx.state.size = style->font.size;
    fsctx.state.align = style->font.align;

    fons_inititer(&fsctx, &iter, 0, 0, string, end, FONS_GLYPH_BITMAP_OPTIONAL);

    while (fons_nextiter(&fsctx, &iter, &q))
    {

        switch (iter.codepoint)
        {

            case 9:
            case 11:
            case 12:
            case 32:
            case 0x00a0:
                type = NVG_SPACE;

                break;

            case 10:
                type = pcodepoint == 13 ? NVG_SPACE : NVG_NEWLINE;

                break;

            case 13:
                type = pcodepoint == 10 ? NVG_SPACE : NVG_NEWLINE;

                break;

            case 0x0085:
                type = NVG_NEWLINE;

                break;

            default:
                if ((iter.codepoint >= 0x4E00 && iter.codepoint <= 0x9FFF) || (iter.codepoint >= 0x3000 && iter.codepoint <= 0x30FF) || (iter.codepoint >= 0xFF00 && iter.codepoint <= 0xFFEF) || (iter.codepoint >= 0x1100 && iter.codepoint <= 0x11FF) || (iter.codepoint >= 0x3130 && iter.codepoint <= 0x318F) || (iter.codepoint >= 0xAC00 && iter.codepoint <= 0xD7AF))
                    type = NVG_CJK_CHAR;
                else
                    type = NVG_CHAR;

                break;

        }

        if (type == NVG_NEWLINE)
        {

            row->start = rowStart != NULL ? rowStart : iter.str;
            row->end = rowEnd != NULL ? rowEnd : iter.str;
            row->width = rowWidth;
            row->minx = rowMinX;
            row->maxx = rowMaxX;
            row->next = iter.next;

            return row->next;

        }

        else
        {

            if (rowStart == NULL)
            {

                if (type == NVG_CHAR || type == NVG_CJK_CHAR)
                {

                    rowStartX = iter.x;
                    rowStart = iter.str;
                    rowEnd = iter.next;
                    rowWidth = iter.nextx - rowStartX;
                    rowMinX = q.x0 - rowStartX;
                    rowMaxX = q.x1 - rowStartX;
                    wordStart = iter.str;
                    breakEnd = rowStart;
                    breakWidth = 0.0;
                    breakMaxX = 0.0;

                }

            }

            else
            {

                float nextWidth = iter.nextx - rowStartX;

                if (type == NVG_CHAR || type == NVG_CJK_CHAR)
                {

                    rowEnd = iter.next;
                    rowWidth = iter.nextx - rowStartX;
                    rowMaxX = q.x1 - rowStartX;

                }

                if (((ptype == NVG_CHAR || ptype == NVG_CJK_CHAR) && type == NVG_SPACE) || type == NVG_CJK_CHAR)
                {

                    breakEnd = iter.str;
                    breakWidth = rowWidth;
                    breakMaxX = rowMaxX;

                }

                if ((ptype == NVG_SPACE && (type == NVG_CHAR || type == NVG_CJK_CHAR)) || type == NVG_CJK_CHAR)
                {

                    wordStart = iter.str;

                }

                if ((type == NVG_CHAR || type == NVG_CJK_CHAR) && nextWidth > width)
                {

                    if (breakEnd == rowStart)
                    {

                        row->start = rowStart;
                        row->end = iter.str;
                        row->width = rowWidth;
                        row->minx = rowMinX;
                        row->maxx = rowMaxX;
                        row->next = iter.str;

                        return row->next;

                    }

                    else
                    {

                        row->start = rowStart;
                        row->end = breakEnd;
                        row->width = breakWidth;
                        row->minx = rowMinX;
                        row->maxx = breakMaxX;
                        row->next = wordStart;

                        return row->next;

                    }

                }

            }

        }

        pcodepoint = iter.codepoint;
        ptype = type;

    }

    if (rowStart != NULL)
    {

        row->start = rowStart;
        row->end = rowEnd;
        row->width = rowWidth;
        row->minx = rowMinX;
        row->maxx = rowMaxX;
        row->next = end;

        return row->next;

    }

    return 0;

}

static float alfi_style_textwidth(struct alfi_style *style, char *text)
{

    struct nvg_textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float w = 0;

    while ((current = calcline(style, current, end, style->box.w, &row)))
    {

        if (w < row.width)
            w = row.width;

    }

    return w;

}

static float alfi_style_textheight(struct alfi_style *style, char *text)
{

    struct nvg_textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    unsigned int i;

    for (i = 0; (current = calcline(style, current, end, style->box.w, &row)); i++);

    return (i) ? i * style->font.size : style->font.size;

}

static void alfi_style_filltext(struct alfi_style *style, char *text)
{

    struct nvg_textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float x = style->box.x;
    float y = style->box.y;

    nvg_setfillcolor(&ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));

    while ((current = calcline(style, current, end, style->box.w, &row)))
    {

        nvg_gl_text(&glctx, &ctx, &ctx.state, &fsctx, x, y, row.start, row.end);

        y += style->font.size;

    }

}

static void alfi_style_filltextinput(struct alfi_style *style, char *text, int offset)
{

    struct nvg_textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float x = style->box.x;
    float y = style->box.y;
    unsigned int i;

    nvg_setfillcolor(&ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));

    for (i = 0; (current = calcline(style, current, end, style->box.w, &row)); i++)
    {

        int length = row.end - row.start;

        if (offset >= 0 && offset <= length)
        {

            x = nvg_gl_text(&glctx, &ctx, &ctx.state, &fsctx, x, y, row.start, row.start + offset);

            nvg_gl_text(&glctx, &ctx, &ctx.state, &fsctx, x, y, row.start + offset, row.end);
            nvgBeginPath(&ctx);
            nvgRoundedRect(&ctx, x, y, 3, style->font.size, 0);
            nvg_setfillcolor(&ctx, nvgRGBA(color_focus.r, color_focus.g, color_focus.b, color_focus.a));
            nvg_gl_fill(&glctx, &ctx, &ctx.state);

            x = style->box.x;

        }

        else
        {

            nvg_gl_text(&glctx, &ctx, &ctx.state, &fsctx, x, y, row.start, row.end);

        }

        offset -= length;
        y += style->font.size;

    }

}

static void alfi_style_filldivider(struct alfi_style *border, struct alfi_style *label, char *content)
{

    nvgBeginPath(&ctx);
    nvgRect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h);
    nvgPathWinding(&ctx, NVG_CW);

    if (strlen(content))
        nvgRect(&ctx, label->box.x - alfi_style_textwidth(label, content) * 0.5 - margin_w, border->box.y, alfi_style_textwidth(label, content) + margin_w * 2, border->box.h);

    nvg_setfillcolor(&ctx, nvgRGBA(border->color.r, border->color.g, border->color.b, border->color.a));
    nvg_gl_fill(&glctx, &ctx, &ctx.state);

    if (strlen(content))
        alfi_style_filltext(label, content);

}

static void alfi_style_fillborder(struct alfi_style *border, struct alfi_style *label, char *content, float bordersize)
{

    nvgBeginPath(&ctx);
    nvgRoundedRect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h, border->box.r);
    nvgPathWinding(&ctx, NVG_CW);
    nvgRoundedRect(&ctx, border->box.x + bordersize, border->box.y + bordersize, border->box.w - bordersize * 2, border->box.h - bordersize * 2, border->box.r);
    nvg_setfillcolor(&ctx, nvgRGBA(border->color.r, border->color.g, border->color.b, border->color.a));
    nvg_gl_fill(&glctx, &ctx, &ctx.state);

    if (strlen(content))
        alfi_style_filltext(label, content);

}

static void alfi_style_fillborder2(struct alfi_style *border, struct alfi_style *label, char *content, float bordersize)
{

    nvgBeginPath(&ctx);
    nvgRoundedRect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h, border->box.r);
    nvgPathWinding(&ctx, NVG_CW);
    nvgRoundedRect(&ctx, border->box.x + bordersize, border->box.y + bordersize, border->box.w - bordersize * 2, border->box.h - bordersize * 2, border->box.r);

    if (strlen(content))
        nvgRect(&ctx, label->box.x - margin_w, border->box.y, alfi_style_textwidth(label, content) + margin_w * 2, bordersize);

    nvg_setfillcolor(&ctx, nvgRGBA(border->color.r, border->color.g, border->color.b, border->color.a));
    nvg_gl_fill(&glctx, &ctx, &ctx.state);

    if (strlen(content))
        alfi_style_filltext(label, content);

}

static void alfi_style_fillrect(struct alfi_style *style)
{

    nvgBeginPath(&ctx);
    nvgRoundedRect(&ctx, style->box.x, style->box.y, style->box.w, style->box.h, style->box.r);
    nvg_setfillcolor(&ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));
    nvg_gl_fill(&glctx, &ctx, &ctx.state);

}

/*
static void alfi_style_paintrect(struct alfi_style *style, struct nvg_paint paint)
{

    nvgBeginPath(&ctx);
    nvgRect(&ctx, style->box.x, style->box.y, style->box.w, style->box.h);
    nvg_setfillpaint(&ctx, paint);
    nvg_gl_fill(&glctx, &ctx, &ctx.state);

}
*/

static void alfi_style_fillcircle(struct alfi_style *style)
{

    nvgBeginPath(&ctx);
    nvgCircle(&ctx, style->box.x, style->box.y, style->box.r);
    nvg_setfillcolor(&ctx, nvgRGBA(style->color.r, style->color.g, style->color.b, style->color.a));
    nvg_gl_fill(&glctx, &ctx, &ctx.state);

}

static void alfi_style_init(struct alfi_style *s)
{

    box_init(&s->box, 0, 0, 0, 0, 0);
    color_init(&s->color, 0, 0, 0, 0);
    font_init(&s->font, 0, 0, 0);

}

static void alfi_style_tween(struct alfi_style *s1, struct alfi_style *s2, float u)
{

    box_lerpfrom(&s1->box, &s2->box, u);
    color_lerpfrom(&s1->color, &s2->color, u);
    font_lerpfrom(&s1->font, &s2->font, u);

}

static void box_expand(struct alfi_box *box, struct alfi_box *child, float px, float py)
{

    if (box->w < child->w + px * 2)
        box->w = child->w + px * 2;

    if (box->h < child->h + py * 2)
        box->h = child->h + py * 2;

}

static void widget_anchor_keyframe(struct alfi_frame_anchor *keyframe, struct alfi_widget_anchor *anchor, unsigned int state, float x, float y, float w, float h)
{

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->label);
    font_init(&keyframe->label.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    color_clone(&keyframe->background.color, &color_background);
    color_clone(&keyframe->label.color, &color_focus);
    box_init(&keyframe->background.box, x, y, w, h, 0);
    box_clone(&keyframe->label.box, &keyframe->background.box);
    box_pad(&keyframe->label.box, margin_w, margin_h);
    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, anchor->label.content));
    box_expand(&keyframe->background.box, &keyframe->label.box, margin_w, margin_h);

}

static void widget_anchor_create(struct alfi_widget *widget)
{

    widget->data.anchor.label.content = parser.createstring(widget->data.anchor.label.content, "");
    widget->data.anchor.link.url = parser.createstring(widget->data.anchor.link.url, "");
    widget->data.anchor.link.mime = parser.createstring(widget->data.anchor.link.mime, "");

}

static void widget_anchor_destroy(struct alfi_widget *widget)
{

    widget->data.anchor.label.content = parser.destroystring(widget->data.anchor.label.content);
    widget->data.anchor.link.url = parser.destroystring(widget->data.anchor.link.url);
    widget->data.anchor.link.mime = parser.destroystring(widget->data.anchor.link.mime);

}

static void widget_anchor_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_anchor keyframe;

    widget_anchor_keyframe(&keyframe, &widget->data.anchor, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.anchor.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.anchor.frame.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->data.anchor.frame.background.box);

}

static void widget_anchor_render(struct alfi_widget *widget)
{

    alfi_style_filltext(&widget->data.anchor.frame.label, widget->data.anchor.label.content);

}

static unsigned int widget_anchor_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_anchor_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

    if (strlen(widget->data.anchor.link.url))
    {

        switch (widget->data.anchor.target.type)
        {

        case ALFI_TARGET_SELF:
            loadpart(widget->data.anchor.link.url);

            break;

        case ALFI_TARGET_BLANK:
            loadmain(widget->data.anchor.link.url);

            break;

        }

    }

}

static unsigned int widget_anchor_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_button_keyframe(struct alfi_frame_button *keyframe, struct alfi_widget_button *button, unsigned int state, float x, float y, float w, float h)
{

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->border);
    alfi_style_init(&keyframe->label);
    font_init(&keyframe->label.font, fontface_bold, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    color_clone(&keyframe->background.color, &color_background);
    color_clone(&keyframe->border.color, &color_focus);
    color_clone(&keyframe->label.color, &color_focustext);
    box_init(&keyframe->background.box, x, y, w, h, 4);
    box_clone(&keyframe->border.box, &keyframe->background.box);
    box_pad(&keyframe->border.box, margin_w, margin_h);
    box_clone(&keyframe->label.box, &keyframe->border.box);
    box_pad(&keyframe->label.box, unit_w - margin_w, unit_h - margin_h);
    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, button->label.content));
    box_expand(&keyframe->border.box, &keyframe->label.box, unit_w - margin_w, unit_h - margin_h);
    box_expand(&keyframe->background.box, &keyframe->border.box, margin_w, margin_h);

}

static void widget_button_create(struct alfi_widget *widget)
{

    widget->data.button.label.content = parser.createstring(widget->data.button.label.content, "undefined");

}

static void widget_button_destroy(struct alfi_widget *widget)
{

    widget->data.button.label.content = parser.destroystring(widget->data.button.label.content);

}

static void widget_button_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_button keyframe;
    float selfh = unit_h * 3;

    widget_button_keyframe(&keyframe, &widget->data.button, widget->state, x, y, w, selfh);
    alfi_style_tween(&widget->data.button.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.button.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.button.frame.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->data.button.frame.background.box);

}

static void widget_button_render(struct alfi_widget *widget)
{

    alfi_style_fillrect(&widget->data.button.frame.border);

    if (strlen(widget->data.button.label.content))
        alfi_style_filltext(&widget->data.button.frame.label, widget->data.button.label.content);

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

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->label);
    font_init(&keyframe->label.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    color_clone(&keyframe->label.color, &color_text);

    if (state == ALFI_STATE_HOVER)
        color_clone(&keyframe->background.color, &color_line);
    else
        color_clone(&keyframe->background.color, &color_background);

    box_init(&keyframe->background.box, x, y, w, h, 4);
    box_clone(&keyframe->label.box, &keyframe->background.box);
    box_pad(&keyframe->label.box, margin_w, margin_h);
    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, choice->label.content));
    box_expand(&keyframe->background.box, &keyframe->label.box, margin_w, margin_h);

}

static void widget_choice_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_choice keyframe;

    widget_choice_keyframe(&keyframe, &widget->data.choice, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.choice.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.choice.frame.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->data.choice.frame.background.box);

}

static void widget_choice_create(struct alfi_widget *widget)
{

    widget->data.choice.label.content = parser.createstring(widget->data.choice.label.content, "");

}

static void widget_choice_destroy(struct alfi_widget *widget)
{

    widget->data.choice.label.content = parser.destroystring(widget->data.choice.label.content);

}

static void widget_choice_render(struct alfi_widget *widget)
{

    alfi_style_fillrect(&widget->data.choice.frame.background);

    if (strlen(widget->data.choice.label.content))
        alfi_style_filltext(&widget->data.choice.frame.label, widget->data.choice.label.content);

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

    struct alfi_widget *parent = pool_findbyname(widget->in.name);

    if (parent)
    {

        switch (parent->type)
        {

        case ALFI_WIDGET_SELECT:
            strcpy(parent->data.select.data.content, widget->data.choice.label.content);

            break;

        }

    }

    setfocus(widget);

}

static unsigned int widget_choice_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_divider_keyframe(struct alfi_frame_divider *keyframe, struct alfi_widget_divider *divider, unsigned int state, float x, float y, float w, float h)
{

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->border);
    alfi_style_init(&keyframe->label);
    font_init(&keyframe->label.font, fontface_regular, fontsize_small, FONS_ALIGN_CENTER | FONS_ALIGN_MIDDLE);
    color_clone(&keyframe->label.color, &color_text);
    color_clone(&keyframe->border.color, &color_line);
    color_clone(&keyframe->background.color, &color_background);
    box_init(&keyframe->background.box, x, y, w, unit_h * 2, 0);
    box_init(&keyframe->border.box, x + margin_w, y + unit_h - 1, w - margin_w * 2, 2, 0);
    box_init(&keyframe->label.box, x, y, w, h, 0);
    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, divider->label.content));
    box_translate(&keyframe->label.box, w * 0.5, unit_h);

}

static void widget_divider_create(struct alfi_widget *widget)
{

    widget->data.divider.label.content = parser.createstring(widget->data.divider.label.content, "");

}

static void widget_divider_destroy(struct alfi_widget *widget)
{

    widget->data.divider.label.content = parser.destroystring(widget->data.divider.label.content);

}

static void widget_divider_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_divider keyframe;

    widget_divider_keyframe(&keyframe, &widget->data.divider, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.divider.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.divider.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.divider.frame.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->data.divider.frame.background.box);

}

static void widget_divider_render(struct alfi_widget *widget)
{

    alfi_style_filldivider(&widget->data.divider.frame.border, &widget->data.divider.frame.label, widget->data.divider.label.content);

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

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->border);
    alfi_style_init(&keyframe->label);
    alfi_style_init(&keyframe->data);

    if (state == ALFI_STATE_FOCUS || strlen(field->data.content))
        font_init(&keyframe->label.font, fontface_regular, fontsize_small, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    else
        font_init(&keyframe->label.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);

    font_init(&keyframe->data.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);

    if (state == ALFI_STATE_FOCUS)
        color_clone(&keyframe->label.color, &color_focus);
    else
        color_clone(&keyframe->label.color, &color_line);

    if (state == ALFI_STATE_FOCUS)
        color_clone(&keyframe->border.color, &color_focus);
    else
        color_clone(&keyframe->border.color, &color_line);

    color_clone(&keyframe->data.color, &color_text);
    color_clone(&keyframe->background.color, &color_background);
    box_init(&keyframe->background.box, x, y, w, h, 4);
    box_clone(&keyframe->border.box, &keyframe->background.box);
    box_clone(&keyframe->label.box, &keyframe->background.box);
    box_clone(&keyframe->data.box, &keyframe->background.box);
    box_pad(&keyframe->border.box, margin_w, margin_h);

    if (state == ALFI_STATE_FOCUS || strlen(field->data.content))
        box_pad(&keyframe->label.box, unit_w, margin_h - keyframe->label.font.size * 0.5);
    else
        box_pad(&keyframe->label.box, unit_w, unit_h);

    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, field->label.content));

    if (state == ALFI_STATE_FOCUS)
        box_pad(&keyframe->data.box, unit_w + margin_w, unit_h + margin_h);
    else
        box_pad(&keyframe->data.box, unit_w, unit_h);

    box_scale(&keyframe->data.box, keyframe->data.box.w, alfi_style_textheight(&keyframe->data, field->data.content));

    if (state == ALFI_STATE_FOCUS)
        box_expand(&keyframe->border.box, &keyframe->data.box, unit_w, unit_h);
    else
        box_expand(&keyframe->border.box, &keyframe->data.box, unit_w - margin_w, unit_h - margin_h);

    box_expand(&keyframe->background.box, &keyframe->border.box, margin_w, margin_h);

}

static void widget_field_create(struct alfi_widget *widget)
{

    widget->data.field.label.content = parser.createstring(widget->data.field.label.content, "");
    widget->data.field.data.content = parser.allocate(widget->data.field.data.content, ALFI_DATASIZE, 1, "");
    widget->data.field.data.total = ALFI_DATASIZE;
    widget->data.field.data.offset = 0;

}

static void widget_field_destroy(struct alfi_widget *widget)
{

    widget->data.field.label.content = parser.destroystring(widget->data.field.label.content);
    widget->data.field.data.content = parser.destroystring(widget->data.field.data.content);

}

static void widget_field_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_field keyframe;

    widget_field_keyframe(&keyframe, &widget->data.field, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.field.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.field.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.field.frame.label, &keyframe.label, u);
    alfi_style_tween(&widget->data.field.frame.data, &keyframe.data, u);
    box_clone(&widget->bb, &widget->data.field.frame.background.box);

}

static void widget_field_render(struct alfi_widget *widget)
{

    if (widget->state == ALFI_STATE_FOCUS || strlen(widget->data.field.data.content))
        alfi_style_fillborder2(&widget->data.field.frame.border, &widget->data.field.frame.label, widget->data.field.label.content, 2.0);
    else
        alfi_style_fillborder(&widget->data.field.frame.border, &widget->data.field.frame.label, widget->data.field.label.content, 2.0);

    if (strlen(widget->data.field.data.content))
    {

        if (widget->state == ALFI_STATE_FOCUS)
            alfi_style_filltextinput(&widget->data.field.frame.data, widget->data.field.data.content, widget->data.field.data.offset);
        else
            alfi_style_filltext(&widget->data.field.frame.data, widget->data.field.data.content);

    }

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

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->label);
    font_init(&keyframe->label.font, fontface_bold, fontsize_xlarge, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    color_clone(&keyframe->background.color, &color_background);
    color_clone(&keyframe->label.color, &color_header);
    box_init(&keyframe->background.box, x, y, w, h, 0);
    box_clone(&keyframe->label.box, &keyframe->background.box);
    box_pad(&keyframe->label.box, margin_w, margin_h);
    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, header->label.content));
    box_expand(&keyframe->background.box, &keyframe->label.box, margin_w, margin_h);

}

static void widget_header_create(struct alfi_widget *widget)
{

    widget->data.header.label.content = parser.createstring(widget->data.header.label.content, "");

}

static void widget_header_destroy(struct alfi_widget *widget)
{

    widget->data.header.label.content = parser.destroystring(widget->data.header.label.content);

}

static void widget_header_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_header keyframe;

    widget_header_keyframe(&keyframe, &widget->data.header, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.header.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.header.frame.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->data.header.frame.background.box);

}

static void widget_header_render(struct alfi_widget *widget)
{

    if (strlen(widget->data.header.label.content))
        alfi_style_filltext(&widget->data.header.frame.label, widget->data.header.label.content);

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
    float cx = 0;
    float cy = 0;
    float cw = 0;
    float ch = h;
    float selfw = w;
    float selfh = h;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y + cy, cw, ch, u);

        cx += child->bb.w;

        if (selfw < cx)
            selfw = cx;

        if (selfh < child->bb.h)
            selfh = child->bb.h;

    }

    box_init(&widget->bb, x, y, selfw, selfh, 0);

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

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->frame);
    color_clone(&keyframe->background.color, &color_background);
    box_init(&keyframe->frame.box, x, y, image->resource.w, image->resource.h, 0);
    box_translate(&keyframe->frame.box, margin_w, margin_h);
    box_clone(&keyframe->background.box, &keyframe->frame.box);
    box_pad(&keyframe->background.box, -margin_w, -margin_h);

}

static void widget_image_create(struct alfi_widget *widget)
{

/*
    loadimage(&widget->data.image.resource, widget->data.image.link.url);
    nvg_gl_imagesize(&glctx, widget->data.image.resource.ref, &widget->data.image.resource.w, &widget->data.image.resource.h);
*/

}

static void widget_image_destroy(struct alfi_widget *widget)
{

/*
    nvg_gl_deletetexture(&glctx, widget->data.image.resource.ref);
*/

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

/*
    struct nvg_paint paint;

    nvgImagePattern(&paint, widget->data.image.frame.frame.box.x, widget->data.image.frame.frame.box.y, widget->data.image.frame.frame.box.w, widget->data.image.frame.frame.box.h, 0.0, widget->data.image.resource.ref, 1.0);
    alfi_style_paintrect(&widget->data.image.frame.frame, paint);
*/

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

    alfi_style_init(&keyframe->dot);
    color_clone(&keyframe->dot.color, &color_header);

    keyframe->dot.box.r = 3.0;

}

static void widget_list_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_list keyframe;
    struct alfi_widget *child = 0;
    float cx = unit_w;
    float cy = 0;
    float cw = w - cx;
    float ch = 0;
    float selfw = w;
    float selfh = h;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y + cy, cw, ch, u);

        cy += child->bb.h;

        if (selfw < child->bb.w)
            selfw = child->bb.w;

        if (selfh < cy)
            selfh = cy;

    }

    widget_list_keyframe(&keyframe, &widget->data.list, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->data.list.frame.dot, &keyframe.dot, u);
    box_init(&widget->bb, x, y, selfw, selfh, 0);

}

static void widget_list_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        call_render(child);

        if (!call_checkflag(child, ALFI_FLAG_ITEM))
            continue;

        widget->data.list.frame.dot.box.x = child->bb.x - margin_w;
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

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->border);
    alfi_style_init(&keyframe->label);
    alfi_style_init(&keyframe->data);

    switch (state)
    {

    case ALFI_STATE_FOCUS:
        font_init(&keyframe->label.font, fontface_regular, fontsize_small, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
        box_init(&keyframe->label.box, x, y, w, h, 0);
        box_translate(&keyframe->label.box, unit_w, 0);
        box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, select->label.content));
        color_clone(&keyframe->label.color, &color_focus);
        font_init(&keyframe->data.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
        box_init(&keyframe->data.box, x, y, w, h, 0);
        box_pad(&keyframe->data.box, unit_w + margin_w, unit_h + margin_h);
        box_scale(&keyframe->data.box, keyframe->data.box.w, alfi_style_textheight(&keyframe->data, select->data.content));
        color_clone(&keyframe->data.color, &color_text);
        box_init(&keyframe->border.box, x, y, w, h, 4);
        box_pad(&keyframe->border.box, margin_w, margin_h);
        color_clone(&keyframe->border.color, &color_focus);
        box_init(&keyframe->background.box, x, y, w, h, 0);
        color_clone(&keyframe->background.color, &color_background);

        break;

    default:
        if (strlen(select->data.content))
        {

            font_init(&keyframe->label.font, fontface_regular, fontsize_small, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
            box_init(&keyframe->label.box, x, y, w, h, 0);
            box_translate(&keyframe->label.box, unit_w, 0);
            box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, select->label.content));
            color_clone(&keyframe->label.color, &color_line);

        }

        else
        {

            font_init(&keyframe->label.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
            box_init(&keyframe->label.box, x, y, w, h, 0);
            box_translate(&keyframe->label.box, unit_w, unit_h);
            box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, select->label.content));
            color_clone(&keyframe->label.color, &color_line);

        }

        font_init(&keyframe->data.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
        box_init(&keyframe->data.box, x, y, w, h, 0);
        box_pad(&keyframe->data.box, unit_w, unit_h);
        box_scale(&keyframe->data.box, keyframe->data.box.w, alfi_style_textheight(&keyframe->data, select->data.content));
        color_clone(&keyframe->data.color, &color_text);
        box_init(&keyframe->border.box, x, y, w, h, 4);
        box_pad(&keyframe->border.box, margin_w, margin_h);
        color_clone(&keyframe->border.color, &color_line);
        box_init(&keyframe->background.box, x, y, w, h, 0);
        color_clone(&keyframe->background.color, &color_background);

        break;

    }

}

static void widget_select_create(struct alfi_widget *widget)
{

    widget->data.select.label.content = parser.createstring(widget->data.select.label.content, "");
    widget->data.select.data.content = parser.allocate(widget->data.select.data.content, ALFI_DATASIZE, 1, "");
    widget->data.select.data.total = ALFI_DATASIZE;
    widget->data.select.data.offset = 0;

}

static void widget_select_destroy(struct alfi_widget *widget)
{

    widget->data.select.label.content = parser.destroystring(widget->data.select.label.content);
    widget->data.select.data.content = parser.destroystring(widget->data.select.data.content);

}

static void widget_select_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_select keyframe;
    struct alfi_widget *child = 0;
    float cx = unit_w;
    float cy = unit_h * 3;
    float cw = w - unit_w * 2;
    float ch = 0;
    float selfw = w;
    float selfh = unit_h * 3;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y + cy, cw, ch, u);

        cy += child->bb.h;

        if (widget->state == ALFI_STATE_FOCUS)
        {

            if (selfw < child->bb.w)
                selfw = child->bb.w;

            if (selfh < cy + unit_h)
                selfh = cy + unit_h;

        }

    }

    widget_select_keyframe(&keyframe, &widget->data.select, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->data.select.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.select.frame.border, &keyframe.border, u);
    alfi_style_tween(&widget->data.select.frame.label, &keyframe.label, u);
    alfi_style_tween(&widget->data.select.frame.data, &keyframe.data, u);
    box_clone(&widget->bb, &widget->data.select.frame.background.box);

}

static void widget_select_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    if (widget->state == ALFI_STATE_FOCUS || strlen(widget->data.select.data.content))
        alfi_style_fillborder2(&widget->data.select.frame.border, &widget->data.select.frame.label, widget->data.select.label.content, 2.0);
    else
        alfi_style_fillborder(&widget->data.select.frame.border, &widget->data.select.frame.label, widget->data.select.label.content, 2.0);

    if (strlen(widget->data.select.data.content))
        alfi_style_filltext(&widget->data.select.frame.data, widget->data.select.data.content);

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

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->label);
    font_init(&keyframe->label.font, fontface_bold, fontsize_large, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    color_clone(&keyframe->background.color, &color_background);
    color_clone(&keyframe->label.color, &color_header);
    box_init(&keyframe->background.box, x, y, w, h, 0);
    box_clone(&keyframe->label.box, &keyframe->background.box);
    box_pad(&keyframe->label.box, margin_w, margin_h);
    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, subheader->label.content));
    box_expand(&keyframe->background.box, &keyframe->label.box, margin_w, margin_h);

}

static void widget_subheader_create(struct alfi_widget *widget)
{

    widget->data.subheader.label.content = parser.createstring(widget->data.subheader.label.content, "");

}

static void widget_subheader_destroy(struct alfi_widget *widget)
{

    widget->data.subheader.label.content = parser.destroystring(widget->data.subheader.label.content);

}

static void widget_subheader_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_subheader keyframe;

    widget_subheader_keyframe(&keyframe, &widget->data.subheader, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.subheader.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.subheader.frame.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->data.subheader.frame.background.box);

}

static void widget_subheader_render(struct alfi_widget *widget)
{

    if (strlen(widget->data.subheader.label.content))
        alfi_style_filltext(&widget->data.subheader.frame.label, widget->data.subheader.label.content);

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
    float cx = 0;
    float cy = 0;
    float cw = unit_w * widget->data.table.grid.clength;
    float ch = unit_h * widget->data.table.grid.rlength;
    float selfw = 0;
    float selfh = 0;
    float rowh = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y + cy, cw, ch, u);

        cx += cw;

        if (rowh < child->bb.h)
            rowh = child->bb.h;

        if (selfw < cx)
            selfw = cx;

        if (cx >= unit_w * widget->data.table.grid.csize)
        {

            cx = 0;

            if (widget->data.table.grid.rlength)
                cy += ch;
            else
                cy += rowh;

            selfh += rowh;
            rowh = 0;

        }

    }

    box_init(&widget->bb, x, y, selfw, selfh + rowh, 0);

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

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->label);
    font_init(&keyframe->label.font, fontface_regular, fontsize_medium, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    color_clone(&keyframe->background.color, &color_background);
    color_clone(&keyframe->label.color, &color_text);
    box_init(&keyframe->background.box, x, y, w, h, 0);
    box_clone(&keyframe->label.box, &keyframe->background.box);
    box_pad(&keyframe->label.box, margin_w, margin_h);
    box_scale(&keyframe->label.box, keyframe->label.box.w, alfi_style_textheight(&keyframe->label, text->label.content));
    box_expand(&keyframe->background.box, &keyframe->label.box, margin_w, margin_h);

}

static void widget_text_create(struct alfi_widget *widget)
{

    widget->data.text.label.content = parser.createstring(widget->data.text.label.content, "");

}

static void widget_text_destroy(struct alfi_widget *widget)
{

    widget->data.text.label.content = parser.destroystring(widget->data.text.label.content);

}

static void widget_text_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_text keyframe;

    widget_text_keyframe(&keyframe, &widget->data.text, widget->state, x, y, w, h);
    alfi_style_tween(&widget->data.text.frame.background, &keyframe.background, u);
    alfi_style_tween(&widget->data.text.frame.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->data.text.frame.background.box);

}

static void widget_text_render(struct alfi_widget *widget)
{

    if (strlen(widget->data.text.label.content))
        alfi_style_filltext(&widget->data.text.frame.label, widget->data.text.label.content);

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
    float cx = 0;
    float cy = 0;
    float cw = w;
    float ch = 0;
    float selfw = w;
    float selfh = h;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y + cy, cw, ch, u);

        cy += child->bb.h;

        if (selfw < child->bb.w)
            selfw = child->bb.w;

        if (selfh < cy)
            selfh = cy;

    }

    box_init(&widget->bb, x, y, selfw, selfh, 0);

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

    alfi_style_init(&keyframe->background);
    color_clone(&keyframe->background.color, &color_background);
    box_init(&keyframe->background.box, x, y, w, h, 0);

}

static void widget_window_create(struct alfi_widget *widget)
{

    widget->data.window.label.content = parser.createstring(widget->data.window.label.content, "undefined");

}

static void widget_window_destroy(struct alfi_widget *widget)
{

    widget->data.window.label.content = parser.destroystring(widget->data.window.label.content);

}

static void widget_window_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_window keyframe;
    struct alfi_widget *child = 0;
    float selfw = unit_w * 28;
    float selfh = unit_h * 28;
    float cx = unit_w * 2;
    float cy = unit_h * 2;
    float cw = selfw - cx * 2;
    float ch = selfh - cy * 2;

    while ((child = pool_nextchild(child, widget)))
    {

        call_place(child, x + cx, y + cy, cw, ch, u);

        if (selfw < child->bb.w + cx * 2)
            selfw = child->bb.w + cx * 2;

        if (selfh < child->bb.h + cy * 2)
            selfh = child->bb.h + cy * 2;

    }

    widget_window_keyframe(&keyframe, &widget->data.window, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->data.window.frame.background, &keyframe.background, u);
    box_init(&widget->bb, x, y, selfw, selfh, 0);

}

static void widget_window_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

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

static struct alfi_widget *findwidget(struct alfi_widget *widget, float x, float y)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        if (box_istouching(&child->bb, x, y))
            return findwidget(child, x, y);

    }

    return widget;

}

static void checktouch(GLFWwindow *window)
{

    struct alfi_widget *main = pool_findbyname("main");
    struct alfi_widget *widget = findwidget(main, mouse_x, mouse_y);

    sethover(widget);

    if (widget)
        setcursor(window, call_getcursor(widget, mouse_x, mouse_y));

}

static void checktitle(GLFWwindow *window)
{

    if (updatetitle)
    {

        struct alfi_widget *widget = pool_findbyname("window");

        if (widget)
            glfwSetWindowTitle(window, widget->data.window.label.content);

        updatetitle = 0;

    }

}

static void render(float x, float y, float w, float h)
{

    struct alfi_widget *window = pool_findbyname("window");
    struct alfi_style background;

    box_init(&background.box, x, y, w, h, 0);
    color_clone(&background.color, &color_background);
    alfi_style_fillrect(&background);

    if (window)
        call_render(window);

}

static void onerror(int error, const char *desc)
{

    printf("GLFW error %d: %s\n", error, desc);

}

static void onwindowsize(GLFWwindow *window, int width, int height)
{

    view_setsize(width, height);
    view_adjust();

}

static void onframebuffersize(GLFWwindow *window, int width, int height)
{

    glViewport(0, 0, width, height);

}

static void onkey(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS)
    {

        switch (key)
        {

        case GLFW_KEY_D:
            if (mods & GLFW_MOD_CONTROL)
                loadmain(res_page_default.url);

            break;

        case GLFW_KEY_H:
            if (mods & GLFW_MOD_CONTROL)
                loadmain(res_page_home.url);

            break;

        case GLFW_KEY_M:
            if (mods & GLFW_MOD_CONTROL)
                setcolors(1);

            break;

        case GLFW_KEY_N:
            if (mods & GLFW_MOD_CONTROL)
                setcolors(0);

            break;

        case GLFW_KEY_Q:
            if (mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(window, GL_TRUE);

            break;

        case GLFW_KEY_R:
            if (mods & GLFW_MOD_CONTROL)
                loadmain(res_page_current.url);

            break;

        case GLFW_KEY_T:
            if (mods & GLFW_MOD_CONTROL)
                loadmain(res_page_test.url);

            break;

        }

    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch (key)
        {

        case GLFW_KEY_UP:
            view_setposition(view_x, view_y + unit_h);
            view_adjust();

            break;

        case GLFW_KEY_DOWN:
            view_setposition(view_x, view_y - unit_h);
            view_adjust();

            break;

        case GLFW_KEY_LEFT:
            if (focus && focus->type == ALFI_WIDGET_FIELD)
            {

                focus->data.field.data.offset--;

                if (focus->data.field.data.offset < 0)
                    focus->data.field.data.offset = 0;

            }

            else
            {

                view_setposition(view_x + unit_w, view_y);
                view_adjust();

            }

            break;

        case GLFW_KEY_RIGHT:
            if (focus && focus->type == ALFI_WIDGET_FIELD)
            {

                unsigned int len = strlen(focus->data.field.data.content);

                focus->data.field.data.offset++;

                if (focus->data.field.data.offset > len)
                    focus->data.field.data.offset = len;

            }

            else
            {

                view_setposition(view_x - unit_w, view_y);
                view_adjust();

            }

            break;

        case GLFW_KEY_PAGE_UP:
            view_setposition(view_x, view_y + view_h / 2);
            view_adjust();

            break;

        case GLFW_KEY_PAGE_DOWN:
            view_setposition(view_x, view_y - view_h / 2);
            view_adjust();

            break;

        case GLFW_KEY_HOME:
            view_setposition(view_x + view_w / 2, view_y);
            view_adjust();

            break;

        case GLFW_KEY_END:
            view_setposition(view_x - view_w / 2, view_y);
            view_adjust();

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
            else
                setfocus(0);

            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (hover)
            {

                hover = parser_destroywidget(hover);

                sethover(hover);

            }

            break;

        }

    }

}

static void oncursor(GLFWwindow *window, double x, double y)
{

    mouse_x = x;
    mouse_y = y;

}

static void onscroll(GLFWwindow *window, double x, double y)
{

    view_setposition(view_x + x * unit_w, view_y + y * unit_h);
    view_adjust();

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

static void setupresources(void)
{

    resource_seturl(&res_page_default, "file://");
    resource_seturl(&res_page_home, "http://blunder.se/");
    resource_seturl(&res_page_test, "file:///usr/share/navi/example.alfi");
    resource_seturl(&res_font_regular, "/usr/share/navi/roboto-regular.ttf");
    resource_seturl(&res_font_bold, "/usr/share/navi/roboto-bold.ttf");
    resource_seturl(&res_font_icon, "/usr/share/navi/icofont.ttf");

}

static void setupfonts(float unit)
{

    fontsize_small = 24;
    fontsize_medium = 32;
    fontsize_large = 48;
    fontsize_xlarge = 96;
    fontface_regular = fons_addfontfile(&fsctx, res_font_regular.url);
    fontface_bold = fons_addfontfile(&fsctx, res_font_bold.url);
    fontface_icon = fons_addfontfile(&fsctx, res_font_icon.url);

}

static void setupcursors(void)
{

    cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursor_ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursor_hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

}

static void update(void)
{

    nvg_gl_beginframe(&glctx, &ctx, view_w, view_h);
    place(view_x, view_y, 0.5);
    view_adjust();
    render(0, 0, view_w, view_h);
    nvg_gl_flush(&glctx);
    nvg_gl_endframe(&glctx);

}

static void run(GLFWwindow *window)
{

    double prevt = glfwGetTime();

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

        glfwPollEvents();
        checktouch(window);
        checktitle(window);

        if (frames)
        {

            update();
            glfwSwapBuffers(window);

        }

    }

}

int main(int argc, char **argv)
{

    GLFWmonitor *monitor;
    GLFWwindow *window;
    const GLFWvidmode *mode; 

    glfwInit();
    glfwSetErrorCallback(onerror);

    monitor = glfwGetPrimaryMonitor();
    mode = glfwGetVideoMode(monitor);
    window = glfwCreateWindow(mode->width, mode->height, "Navi 0.1", 0, 0);

    view_setsize(mode->width, mode->height);
    glfwSetWindowSizeCallback(window, onwindowsize);
    glfwSetFramebufferSizeCallback(window, onframebuffersize);
    glfwSetKeyCallback(window, onkey);
    glfwSetMouseButtonCallback(window, onbutton);
    glfwSetCursorPosCallback(window, oncursor);
    glfwSetScrollCallback(window, onscroll);
    glfwSetCharCallback(window, onchar);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetTime(0);
    glewInit();
    fons_create(&fsctx, 512, 512, FONS_ZERO_TOPLEFT);
    nvg_gl_create(&glctx, fsctx.width, fsctx.height);
    parser_init(&parser, parser_fail, parser_findwidget, parser_createwidget, parser_destroywidget, parser_allocate, parser_createstring, parser_destroystring);
    call_register(ALFI_WIDGET_ANCHOR, ALFI_FLAG_ITEM, widget_anchor_create, widget_anchor_destroy, widget_anchor_place, widget_anchor_render, widget_anchor_setstate, widget_anchor_onclick, widget_anchor_getcursor);
    call_register(ALFI_WIDGET_BUTTON, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_button_create, widget_button_destroy, widget_button_place, widget_button_render, widget_button_setstate, widget_button_onclick, widget_button_getcursor);
    call_register(ALFI_WIDGET_CHOICE, ALFI_FLAG_CONTAINER, widget_choice_create, widget_choice_destroy, widget_choice_place, widget_choice_render, widget_choice_setstate, widget_choice_onclick, widget_choice_getcursor);
    call_register(ALFI_WIDGET_DIVIDER, ALFI_FLAG_ITEM, widget_divider_create, widget_divider_destroy, widget_divider_place, widget_divider_render, widget_divider_setstate, widget_divider_onclick, widget_divider_getcursor);
    call_register(ALFI_WIDGET_HSTACK, ALFI_FLAG_CONTAINER, 0, 0, widget_hstack_place, widget_hstack_render, widget_hstack_setstate, widget_hstack_onclick, widget_hstack_getcursor);
    call_register(ALFI_WIDGET_FIELD, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_field_create, widget_field_destroy, widget_field_place, widget_field_render, widget_field_setstate, widget_field_onclick, widget_field_getcursor);
    call_register(ALFI_WIDGET_HEADER, ALFI_FLAG_ITEM, widget_header_create, widget_header_destroy, widget_header_place, widget_header_render, widget_header_setstate, widget_header_onclick, widget_header_getcursor);
    call_register(ALFI_WIDGET_IMAGE, ALFI_FLAG_ITEM, widget_image_create, widget_image_destroy, widget_image_place, widget_image_render, widget_image_setstate, widget_image_onclick, widget_image_getcursor);
    call_register(ALFI_WIDGET_LIST, ALFI_FLAG_CONTAINER, 0, 0, widget_list_place, widget_list_render, widget_list_setstate, widget_list_onclick, widget_list_getcursor);
    call_register(ALFI_WIDGET_SELECT, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_select_create, widget_select_destroy, widget_select_place, widget_select_render, widget_select_setstate, widget_select_onclick, widget_select_getcursor);
    call_register(ALFI_WIDGET_SUBHEADER, ALFI_FLAG_ITEM, widget_subheader_create, widget_subheader_destroy, widget_subheader_place, widget_subheader_render, widget_subheader_setstate, widget_subheader_onclick, widget_subheader_getcursor);
    call_register(ALFI_WIDGET_TABLE, ALFI_FLAG_CONTAINER, 0, 0, widget_table_place, widget_table_render, widget_table_setstate, widget_table_onclick, widget_table_getcursor);
    call_register(ALFI_WIDGET_TEXT, ALFI_FLAG_ITEM, widget_text_create, widget_text_destroy, widget_text_place, widget_text_render, widget_text_setstate, widget_text_onclick, widget_text_getcursor);
    call_register(ALFI_WIDGET_VSTACK, ALFI_FLAG_CONTAINER, 0, 0, widget_vstack_place, widget_vstack_render, widget_vstack_setstate, widget_vstack_onclick, widget_vstack_getcursor);
    call_register(ALFI_WIDGET_WINDOW, ALFI_FLAG_CONTAINER, widget_window_create, widget_window_destroy, widget_window_place, widget_window_render, widget_window_setstate, widget_window_onclick, widget_window_getcursor);
    pool_init();
    setupresources();
    setupcursors();
    setupfonts(unit_h);
    setcolors(0);
    glfwPollEvents();
    update();
    glfwSwapBuffers(window);
    loadbase();
    loadmain(res_page_default.url);
    run(window);
    nvg_gl_delete(&glctx);
    fons_delete(&fsctx);
    glfwTerminate();

    return 0;

}

