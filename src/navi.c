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
static unsigned int page_w;
static unsigned int page_h;
static unsigned int unit_w;
static unsigned int unit_h;
static unsigned int margin_w;
static unsigned int margin_h;
static int scroll_x;
static int scroll_y;
static double mouse_x;
static double mouse_y;
static int fontface_regular;
static int fontface_bold;
static int fontface_icon;
static unsigned int fontsize_small;
static unsigned int fontsize_medium;
static unsigned int fontsize_large;
static unsigned int fontsize_xlarge;
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
static struct resource res_page;
static struct resource res_font_regular;
static struct resource res_font_bold;
static struct resource res_font_icon;
static unsigned int updatetitle;
static unsigned int historyindex;

enum chartype
{

    CHARTYPE_SPACE,
    CHARTYPE_NEWLINE,
    CHARTYPE_CHAR,
    CHARTYPE_CJKCHAR

};

struct textrow
{

    const char *start;
    const char *end;
    const char *next;
    float width;
    float minx;
    float maxx;

};

static unsigned int stringappend(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    if (offset >= ocount)
        return 0;

    if (icount > ocount - offset)
        icount = ocount - offset;

    memcpy(op + offset, ip, icount);

    return icount;

}

static unsigned int buildkeyvalue(void *buffer, unsigned int count, unsigned int offset, char *key, char *value)
{

    if (offset)
        offset += stringappend(buffer, count, "&", 1, offset);

    offset += stringappend(buffer, count, key, strlen(key), offset);
    offset += stringappend(buffer, count, "=", 1, offset);
    offset += stringappend(buffer, count, value, strlen(value), offset);

    return offset;

}

static unsigned int builddata(void *buffer, unsigned int count)
{

    struct alfi_widget *widget = 0;
    unsigned int offset = 0;

    while ((widget = pool_next(widget)))
    {

        if (!strlen(widget->header.id.name))
            continue;

        switch (widget->header.type)
        {

        case ALFI_WIDGET_FIELD:
            offset += buildkeyvalue(buffer, count, offset, widget->header.id.name, widget->payload.field.data.content);

            break;

        case ALFI_WIDGET_SELECT:
            offset += buildkeyvalue(buffer, count, offset, widget->header.id.name, widget->payload.select.data.content);

            break;

        }

    }

    offset += stringappend(buffer, count, "", 1, offset);

    return offset;

}

static struct alfi_widget *prevflag(struct alfi_widget *widget, unsigned int flag)
{

    while ((widget = pool_prev(widget)))
    {

        if (call_checkflag(widget, flag))
            return widget;

    }

    return 0;

}

static struct alfi_widget *nextflag(struct alfi_widget *widget, unsigned int flag)
{

    while ((widget = pool_next(widget)))
    {

        if (call_checkflag(widget, flag))
            return widget;

    }

    return 0;

}

static char *createstring(char *string, char *content)
{

    return pool_allocate(string, strlen(content) + 1, strlen(content) + 1, content);

}

static char *destroystring(char *string)
{

    return pool_allocate(string, 0, 0, 0);

}

static struct alfi_widget *parser_createwidget(unsigned int type, char *in)
{

    struct alfi_widget *widget = pool_create();

    memset(&widget->payload, 0, sizeof (union alfi_payload));
    memset(&widget->frame, 0, sizeof (union alfi_frame));

    widget->header.type = type;
    widget->header.id.name = createstring(widget->header.id.name, "");
    widget->header.in.name = createstring(widget->header.in.name, in);

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

    widget->header.id.name = destroystring(widget->header.id.name);
    widget->header.in.name = destroystring(widget->header.in.name);

    call_destroy(widget);
    pool_destroy(widget);

    return 0;

}

static void parser_fail(void)
{

    printf("Parsing failed on line %u\n", parser.expr.line + 1);
    exit(EXIT_FAILURE);

}

static void configure(int w, int h)
{

    page_w = w;
    page_h = h;
    unit_w = (w > 1024) ? w / 32 : 32;
    unit_h = (h > 1024) ? h / 32 : 32;
    margin_w = 12;
    margin_h = 12;
    fontsize_small = 24;
    fontsize_medium = 32;
    fontsize_large = 48;
    fontsize_xlarge = 96;

}

static void resetscroll(void)
{

    scroll_x = 0;
    scroll_y = 0;

}

static void scroll(int x, int y)
{

    scroll_x += x * unit_w;
    scroll_y += y * unit_h;

}

static void adjust(float w, float h)
{

    if (page_w < w)
    {

        if (scroll_x > 0)
            scroll_x = 0;

        if (scroll_x < page_w - w)
            scroll_x = page_w - w;

    }

    else
    {

        scroll_x = page_w / 2 - w / 2;

    }

    if (page_h < h)
    {

        if (scroll_y > 0)
            scroll_y = 0;

        if (scroll_y < page_h - h)
            scroll_y = page_h - h;

    }

    else
    {

        scroll_y = 0;

    }

}

static void theme(unsigned int id)
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

        widget->header.id.name = createstring(widget->header.id.name, "window");
        widget->payload.window.label.content = createstring(widget->payload.window.label.content, "undefined");

    }

    widget = parser_createwidget(ALFI_WIDGET_VSTACK, "window");

    if (widget)
    {

        widget->header.id.name = createstring(widget->header.id.name, "main");
        widget->payload.vstack.halign.direction = ALFI_HALIGN_LEFT;
        widget->payload.vstack.valign.direction = ALFI_VALIGN_TOP;

    }

}

static void clear(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
        child = parser_destroywidget(child);

}

static void loadalfi(char *url, void *data, unsigned int count)
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

        if (count)
            execlp("navi-resolve", "navi-resolve", url, data, NULL);
        else
            execlp("navi-resolve", "navi-resolve", url, NULL);

        exit(EXIT_FAILURE);

    }

}

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

            resource->ref = nvg_gl_texture_create(&glctx, NVG_TEXTURE_RGBA, w, h, 0, img);

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

static void loadresources(void)
{

    struct alfi_widget *widget = 0;

    while ((widget = pool_next(widget)))
    {

        switch (widget->header.type)
        {

        case ALFI_WIDGET_IMAGE:
            loadimage(&widget->payload.image.resource, widget->payload.image.link.url);

            {

                struct nvg_gl_texture *tex = nvg_gl_findtexture(&glctx, widget->payload.image.resource.ref);

                widget->payload.image.resource.w = tex->width;
                widget->payload.image.resource.h = tex->height;

            }

            break;

        }

    }

}

static void unloadresources(void)
{

    struct alfi_widget *widget = 0;

    while ((widget = pool_next(widget)))
    {

        switch (widget->header.type)
        {

        case ALFI_WIDGET_IMAGE:
            nvg_gl_texture_destroy(&glctx, widget->payload.image.resource.ref);

            break;

        }

    }

}

static void loadself(char *url, void *data, unsigned int count)
{

    struct alfi_widget *root = pool_findbyname("window");
    struct resource res;

    resource_seturl(&res, url);
    /* Move */
    unloadresources();
    loadalfi(res.url, data, count);
    /* Move */
    loadresources();
    call_place(root, scroll_x, scroll_y, 0, 0, 1.0);

    updatetitle = 1;

}

static void loadblank(char *url, void *data, unsigned int count)
{

    struct alfi_widget *root = pool_findbyname("window");
    struct alfi_widget *main = pool_findbyname("main");

    resource_seturl(&res_page, url);
    clear(main);
    resetscroll();
    adjust(root->bb.w, root->bb.h);
    loadself(res_page.url, data, count);

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

static const char *calcline(struct alfi_style *style, const char *string, const char *end, float width, struct textrow *row)
{

    struct fons_textiter iter;
    struct fons_quad q;
    float rowStartX = 0;
    float rowWidth = 0;
    float rowMinX = 0;
    float rowMaxX = 0;
    const char *rowStart = 0;
    const char *rowEnd = 0;
    const char *wordStart = 0;
    const char *breakEnd = 0;
    float breakWidth = 0;
    float breakMaxX = 0;
    int type = CHARTYPE_SPACE;
    int ptype = CHARTYPE_SPACE;
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
                type = CHARTYPE_SPACE;

                break;

            case 10:
                type = pcodepoint == 13 ? CHARTYPE_SPACE : CHARTYPE_NEWLINE;

                break;

            case 13:
                type = pcodepoint == 10 ? CHARTYPE_SPACE : CHARTYPE_NEWLINE;

                break;

            case 0x0085:
                type = CHARTYPE_NEWLINE;

                break;

            default:
                if ((iter.codepoint >= 0x4E00 && iter.codepoint <= 0x9FFF) || (iter.codepoint >= 0x3000 && iter.codepoint <= 0x30FF) || (iter.codepoint >= 0xFF00 && iter.codepoint <= 0xFFEF) || (iter.codepoint >= 0x1100 && iter.codepoint <= 0x11FF) || (iter.codepoint >= 0x3130 && iter.codepoint <= 0x318F) || (iter.codepoint >= 0xAC00 && iter.codepoint <= 0xD7AF))
                    type = CHARTYPE_CJKCHAR;
                else
                    type = CHARTYPE_CHAR;

                break;

        }

        if (type == CHARTYPE_NEWLINE)
        {

            row->start = rowStart != 0 ? rowStart : iter.str;
            row->end = rowEnd != 0 ? rowEnd : iter.str;
            row->width = rowWidth;
            row->minx = rowMinX;
            row->maxx = rowMaxX;
            row->next = iter.next;

            return row->next;

        }

        else
        {

            if (rowStart == 0)
            {

                if (type == CHARTYPE_CHAR || type == CHARTYPE_CJKCHAR)
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

                if (type == CHARTYPE_CHAR || type == CHARTYPE_CJKCHAR)
                {

                    rowEnd = iter.next;
                    rowWidth = iter.nextx - rowStartX;
                    rowMaxX = q.x1 - rowStartX;

                }

                if (((ptype == CHARTYPE_CHAR || ptype == CHARTYPE_CJKCHAR) && type == CHARTYPE_SPACE) || type == CHARTYPE_CJKCHAR)
                {

                    breakEnd = iter.str;
                    breakWidth = rowWidth;
                    breakMaxX = rowMaxX;

                }

                if ((ptype == CHARTYPE_SPACE && (type == CHARTYPE_CHAR || type == CHARTYPE_CJKCHAR)) || type == CHARTYPE_CJKCHAR)
                {

                    wordStart = iter.str;

                }

                if ((type == CHARTYPE_CHAR || type == CHARTYPE_CJKCHAR) && nextWidth > width)
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

    if (rowStart != 0)
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

    struct textrow row;
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

    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    unsigned int i;

    for (i = 0; (current = calcline(style, current, end, style->box.w, &row)); i++);

    return (i) ? i * style->font.size : style->font.size;

}

void renderfill(struct nvg_paint *paint, struct nvg_scissor *scissor)
{

    nvg_flatten(&ctx);
    nvg_expand(&ctx);
    nvg_gl_render_paths(&glctx, paint, scissor, ctx.bounds, ctx.paths, ctx.npaths);

}

static float rendertext(struct nvg_paint *paint, struct nvg_scissor *scissor, float *xform, float x, float y, const char *string, const char *end)
{

    struct fons_textiter iter;
    struct fons_quad q;
    int cverts = (end - string) * 6;
    int nverts = 0;
    int dirty[4];

    fons_inititer(&fsctx, &iter, x, y, string, end, FONS_GLYPH_BITMAP_REQUIRED);

    while (fons_nextiter(&fsctx, &iter, &q))
    {

        float c[8];

        nvg_getpoints(&c[0], &c[1], xform, q.x0, q.y0);
        nvg_getpoints(&c[2], &c[3], xform, q.x1, q.y0);
        nvg_getpoints(&c[4], &c[5], xform, q.x1, q.y1);
        nvg_getpoints(&c[6], &c[7], xform, q.x0, q.y1);

        if (nverts + 6 <= cverts)
        {

            nvg_setvertex(&ctx.verts[nverts + 0], c[0], c[1], q.s0, q.t0);
            nvg_setvertex(&ctx.verts[nverts + 1], c[4], c[5], q.s1, q.t1);
            nvg_setvertex(&ctx.verts[nverts + 2], c[2], c[3], q.s1, q.t0);
            nvg_setvertex(&ctx.verts[nverts + 3], c[0], c[1], q.s0, q.t0);
            nvg_setvertex(&ctx.verts[nverts + 4], c[6], c[7], q.s0, q.t1);
            nvg_setvertex(&ctx.verts[nverts + 5], c[4], c[5], q.s1, q.t1);

            nverts += 6;

        }

    }

    if (fons_validate(&fsctx, dirty))
        nvg_gl_texture_update(&glctx, glctx.fontimage, dirty[0], dirty[1], dirty[2] - dirty[0], dirty[3] - dirty[1], fsctx.texdata);

    paint->image = glctx.fontimage;

    nvg_gl_render_vertices(&glctx, paint, scissor, ctx.verts, nverts);

    return iter.nextx;

}

static void alfi_style_filltext(struct alfi_style *style, char *text)
{

    struct nvg_scissor scissor;
    struct nvg_paint textcolor;
    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float x = style->box.x;
    float y = style->box.y;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&textcolor, style->color.r, style->color.g, style->color.b, style->color.a);

    while ((current = calcline(style, current, end, style->box.w, &row)))
    {

        rendertext(&textcolor, &scissor, ctx.xform, x, y, row.start, row.end);

        y += style->font.size;

    }

}

static void alfi_style_filltextinput(struct alfi_style *style, char *text, int offset)
{

    struct nvg_scissor scissor;
    struct nvg_paint textcolor;
    struct nvg_paint cursorcolor;
    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float x = style->box.x;
    float y = style->box.y;
    unsigned int i;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&textcolor, style->color.r, style->color.g, style->color.b, style->color.a);
    nvg_paint_color(&cursorcolor, color_focus.r, color_focus.g, color_focus.b, color_focus.a);

    for (i = 0; (current = calcline(style, current, end, style->box.w, &row)); i++)
    {

        int length = row.end - row.start;

        if (offset >= 0 && offset <= length)
        {

            x = rendertext(&textcolor, &scissor, ctx.xform, x, y, row.start, row.start + offset);

            rendertext(&textcolor, &scissor, ctx.xform, x, y, row.start + offset, row.end);
            nvg_path_begin(&ctx);
            nvg_path_roundedrect(&ctx, x, y, 3, style->font.size, 0);
            renderfill(&cursorcolor, &scissor);

            x = style->box.x;

        }

        else
        {

            rendertext(&textcolor, &scissor, ctx.xform, x, y, row.start, row.end);

        }

        offset -= length;
        y += style->font.size;

    }

}

static void alfi_style_filldivider(struct alfi_style *border, struct alfi_style *label, char *content)
{

    struct nvg_scissor scissor;
    struct nvg_paint bordercolor;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&bordercolor, border->color.r, border->color.g, border->color.b, border->color.a);
    nvg_path_begin(&ctx);
    nvg_path_rect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h);
    nvg_path_hole(&ctx);

    if (strlen(content))
        nvg_path_rect(&ctx, label->box.x - alfi_style_textwidth(label, content) * 0.5 - margin_w, border->box.y, alfi_style_textwidth(label, content) + margin_w * 2, border->box.h);

    renderfill(&bordercolor, &scissor);

    if (strlen(content))
        alfi_style_filltext(label, content);

}

static void alfi_style_fillborder(struct alfi_style *border, struct alfi_style *label, char *content, float bordersize)
{

    struct nvg_scissor scissor;
    struct nvg_paint bordercolor;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&bordercolor, border->color.r, border->color.g, border->color.b, border->color.a);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h, border->box.r);
    nvg_path_hole(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x + bordersize, border->box.y + bordersize, border->box.w - bordersize * 2, border->box.h - bordersize * 2, border->box.r);
    renderfill(&bordercolor, &scissor);

    if (strlen(content))
        alfi_style_filltext(label, content);

}

static void alfi_style_fillborder2(struct alfi_style *border, struct alfi_style *label, char *content, float bordersize)
{

    struct nvg_scissor scissor;
    struct nvg_paint bordercolor;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&bordercolor, border->color.r, border->color.g, border->color.b, border->color.a);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h, border->box.r);
    nvg_path_hole(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x + bordersize, border->box.y + bordersize, border->box.w - bordersize * 2, border->box.h - bordersize * 2, border->box.r);

    if (strlen(content))
        nvg_path_rect(&ctx, label->box.x - margin_w, border->box.y, alfi_style_textwidth(label, content) + margin_w * 2, bordersize);

    renderfill(&bordercolor, &scissor);

    if (strlen(content))
        alfi_style_filltext(label, content);

}

static void alfi_style_paintrect(struct alfi_style *style, struct nvg_paint *paint)
{

    struct nvg_scissor scissor;

    nvg_scissor_init(&scissor);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, style->box.x, style->box.y, style->box.w, style->box.h, style->box.r);
    renderfill(paint, &scissor);

}

static void alfi_style_fillrect(struct alfi_style *style)
{

    struct nvg_paint color;

    nvg_paint_color(&color, style->color.r, style->color.g, style->color.b, style->color.a);
    alfi_style_paintrect(style, &color);

}

static void alfi_style_fillcircle(struct alfi_style *style)
{

    struct nvg_scissor scissor;
    struct nvg_paint color;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&color, style->color.r, style->color.g, style->color.b, style->color.a);
    nvg_path_begin(&ctx);
    nvg_path_circle(&ctx, style->box.x, style->box.y, style->box.r);
    renderfill(&color, &scissor);

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

static void widget_anchor_keyframe(struct alfi_frame_anchor *keyframe, struct alfi_payload_anchor *anchor, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.anchor.label.content = createstring(widget->payload.anchor.label.content, "");
    widget->payload.anchor.link.url = createstring(widget->payload.anchor.link.url, "");
    widget->payload.anchor.link.mime = createstring(widget->payload.anchor.link.mime, "");

}

static void widget_anchor_destroy(struct alfi_widget *widget)
{

    widget->payload.anchor.label.content = destroystring(widget->payload.anchor.label.content);
    widget->payload.anchor.link.url = destroystring(widget->payload.anchor.link.url);
    widget->payload.anchor.link.mime = destroystring(widget->payload.anchor.link.mime);

}

static void widget_anchor_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_anchor keyframe;

    widget_anchor_keyframe(&keyframe, &widget->payload.anchor, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.anchor.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.anchor.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->frame.anchor.background.box);

}

static void widget_anchor_render(struct alfi_widget *widget)
{

    alfi_style_filltext(&widget->frame.anchor.label, widget->payload.anchor.label.content);

}

static unsigned int widget_anchor_setstate(struct alfi_widget *widget, unsigned int state)
{

    return ALFI_STATE_NORMAL;

}

static void widget_anchor_onclick(struct alfi_widget *widget)
{

    setfocus(widget);

    if (strlen(widget->payload.anchor.link.url))
    {

        switch (widget->payload.anchor.target.type)
        {

        case ALFI_TARGET_SELF:
            loadself(widget->payload.anchor.link.url, 0, 0);

            break;

        case ALFI_TARGET_BLANK:
            loadblank(widget->payload.anchor.link.url, 0, 0);

            break;

        }

    }

}

static unsigned int widget_anchor_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_button_keyframe(struct alfi_frame_button *keyframe, struct alfi_payload_button *button, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.button.label.content = createstring(widget->payload.button.label.content, "undefined");
    widget->payload.anchor.link.url = createstring(widget->payload.anchor.link.url, "");
    widget->payload.anchor.link.mime = createstring(widget->payload.anchor.link.mime, "");

}

static void widget_button_destroy(struct alfi_widget *widget)
{

    widget->payload.button.label.content = destroystring(widget->payload.button.label.content);
    widget->payload.anchor.link.url = destroystring(widget->payload.anchor.link.url);
    widget->payload.anchor.link.mime = destroystring(widget->payload.anchor.link.mime);

}

static void widget_button_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_button keyframe;
    float selfh = unit_h * 3;

    widget_button_keyframe(&keyframe, &widget->payload.button, widget->state, x, y, w, selfh);
    alfi_style_tween(&widget->frame.button.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.button.border, &keyframe.border, u);
    alfi_style_tween(&widget->frame.button.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->frame.button.background.box);

}

static void widget_button_render(struct alfi_widget *widget)
{

    alfi_style_fillrect(&widget->frame.button.border);

    if (strlen(widget->payload.button.label.content))
        alfi_style_filltext(&widget->frame.button.label, widget->payload.button.label.content);

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

    char data[4096];
    unsigned int count = builddata(data, 4096);

    printf("%s\n", (char *)data);
    setfocus(widget);

    if (strlen(widget->payload.button.link.url))
    {

        switch (widget->payload.button.target.type)
        {

        case ALFI_TARGET_SELF:
            loadself(widget->payload.button.link.url, data, count);

            break;

        case ALFI_TARGET_BLANK:
            loadblank(widget->payload.button.link.url, data, count);

            break;

        }

    }

}

static unsigned int widget_button_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_choice_keyframe(struct alfi_frame_choice *keyframe, struct alfi_payload_choice *choice, unsigned int state, float x, float y, float w, float h)
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

    widget_choice_keyframe(&keyframe, &widget->payload.choice, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.choice.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.choice.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->frame.choice.background.box);

}

static void widget_choice_create(struct alfi_widget *widget)
{

    widget->payload.choice.label.content = createstring(widget->payload.choice.label.content, "");

}

static void widget_choice_destroy(struct alfi_widget *widget)
{

    widget->payload.choice.label.content = destroystring(widget->payload.choice.label.content);

}

static void widget_choice_render(struct alfi_widget *widget)
{

    alfi_style_fillrect(&widget->frame.choice.background);

    if (strlen(widget->payload.choice.label.content))
        alfi_style_filltext(&widget->frame.choice.label, widget->payload.choice.label.content);

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

    struct alfi_widget *parent = pool_findbyname(widget->header.in.name);

    if (parent)
    {

        switch (parent->header.type)
        {

        case ALFI_WIDGET_SELECT:
            strcpy(parent->payload.select.data.content, widget->payload.choice.label.content);

            break;

        }

    }

    setfocus(widget);

}

static unsigned int widget_choice_getcursor(struct alfi_widget *widget, float x, float y)
{

    return ALFI_CURSOR_HAND;

}

static void widget_divider_keyframe(struct alfi_frame_divider *keyframe, struct alfi_payload_divider *divider, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.divider.label.content = createstring(widget->payload.divider.label.content, "");

}

static void widget_divider_destroy(struct alfi_widget *widget)
{

    widget->payload.divider.label.content = destroystring(widget->payload.divider.label.content);

}

static void widget_divider_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_divider keyframe;

    widget_divider_keyframe(&keyframe, &widget->payload.divider, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.divider.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.divider.border, &keyframe.border, u);
    alfi_style_tween(&widget->frame.divider.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->frame.divider.background.box);

}

static void widget_divider_render(struct alfi_widget *widget)
{

    alfi_style_filldivider(&widget->frame.divider.border, &widget->frame.divider.label, widget->payload.divider.label.content);

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

static void widget_field_keyframe(struct alfi_frame_field *keyframe, struct alfi_payload_field *field, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.field.label.content = createstring(widget->payload.field.label.content, "");
    widget->payload.field.data.content = parser.allocate(widget->payload.field.data.content, ALFI_DATASIZE, 1, "");
    widget->payload.field.data.total = ALFI_DATASIZE;
    widget->payload.field.data.offset = 0;

}

static void widget_field_destroy(struct alfi_widget *widget)
{

    widget->payload.field.label.content = destroystring(widget->payload.field.label.content);
    widget->payload.field.data.content = destroystring(widget->payload.field.data.content);

}

static void widget_field_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_field keyframe;

    widget_field_keyframe(&keyframe, &widget->payload.field, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.field.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.field.border, &keyframe.border, u);
    alfi_style_tween(&widget->frame.field.label, &keyframe.label, u);
    alfi_style_tween(&widget->frame.field.data, &keyframe.data, u);
    box_clone(&widget->bb, &widget->frame.field.background.box);

}

static void widget_field_render(struct alfi_widget *widget)
{

    if (widget->state == ALFI_STATE_FOCUS || strlen(widget->payload.field.data.content))
        alfi_style_fillborder2(&widget->frame.field.border, &widget->frame.field.label, widget->payload.field.label.content, 2.0);
    else
        alfi_style_fillborder(&widget->frame.field.border, &widget->frame.field.label, widget->payload.field.label.content, 2.0);

    if (strlen(widget->payload.field.data.content))
    {

        if (widget->state == ALFI_STATE_FOCUS)
            alfi_style_filltextinput(&widget->frame.field.data, widget->payload.field.data.content, widget->payload.field.data.offset);
        else
            alfi_style_filltext(&widget->frame.field.data, widget->payload.field.data.content);

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

static void widget_header_keyframe(struct alfi_frame_header *keyframe, struct alfi_payload_header *header, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.header.label.content = createstring(widget->payload.header.label.content, "");

}

static void widget_header_destroy(struct alfi_widget *widget)
{

    widget->payload.header.label.content = destroystring(widget->payload.header.label.content);

}

static void widget_header_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_header keyframe;

    widget_header_keyframe(&keyframe, &widget->payload.header, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.header.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.header.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->frame.header.background.box);

}

static void widget_header_render(struct alfi_widget *widget)
{

    if (strlen(widget->payload.header.label.content))
        alfi_style_filltext(&widget->frame.header.label, widget->payload.header.label.content);

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

static void widget_image_keyframe(struct alfi_frame_image *keyframe, struct alfi_payload_image *image, unsigned int state, float x, float y, float w, float h)
{

    alfi_style_init(&keyframe->background);
    alfi_style_init(&keyframe->frame);
    color_clone(&keyframe->background.color, &color_background);
    box_init(&keyframe->frame.box, x, y, image->resource.w, image->resource.h, 0);
    box_translate(&keyframe->frame.box, margin_w, margin_h);
    box_clone(&keyframe->background.box, &keyframe->frame.box);
    box_pad(&keyframe->background.box, -((int)margin_w), -((int)margin_h));

}

static void widget_image_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_image keyframe;

    widget_image_keyframe(&keyframe, &widget->payload.image, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.image.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.image.frame, &keyframe.frame, u);
    box_clone(&widget->bb, &widget->frame.image.background.box);

}

static void widget_image_render(struct alfi_widget *widget)
{

    struct nvg_paint paint;

    nvg_paint_image(&paint, widget->frame.image.frame.box.x, widget->frame.image.frame.box.y, widget->frame.image.frame.box.w, widget->frame.image.frame.box.h, 0.0, widget->payload.image.resource.ref, 1.0);
    alfi_style_paintrect(&widget->frame.image.frame, &paint);

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

static void widget_list_keyframe(struct alfi_frame_list *keyframe, struct alfi_payload_list *list, unsigned int state, float x, float y, float w, float h)
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

    widget_list_keyframe(&keyframe, &widget->payload.list, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->frame.list.dot, &keyframe.dot, u);
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

        widget->frame.list.dot.box.x = child->bb.x - margin_w;
        widget->frame.list.dot.box.y = child->bb.y + child->bb.h * 0.5;

        alfi_style_fillcircle(&widget->frame.list.dot);

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

static void widget_select_keyframe(struct alfi_frame_select *keyframe, struct alfi_payload_select *select, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.select.label.content = createstring(widget->payload.select.label.content, "");
    widget->payload.select.data.content = parser.allocate(widget->payload.select.data.content, ALFI_DATASIZE, 1, "");
    widget->payload.select.data.total = ALFI_DATASIZE;
    widget->payload.select.data.offset = 0;

}

static void widget_select_destroy(struct alfi_widget *widget)
{

    widget->payload.select.label.content = destroystring(widget->payload.select.label.content);
    widget->payload.select.data.content = destroystring(widget->payload.select.data.content);

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

    widget_select_keyframe(&keyframe, &widget->payload.select, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->frame.select.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.select.border, &keyframe.border, u);
    alfi_style_tween(&widget->frame.select.label, &keyframe.label, u);
    alfi_style_tween(&widget->frame.select.data, &keyframe.data, u);
    box_clone(&widget->bb, &widget->frame.select.background.box);

}

static void widget_select_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    if (widget->state == ALFI_STATE_FOCUS || strlen(widget->payload.select.data.content))
        alfi_style_fillborder2(&widget->frame.select.border, &widget->frame.select.label, widget->payload.select.label.content, 2.0);
    else
        alfi_style_fillborder(&widget->frame.select.border, &widget->frame.select.label, widget->payload.select.label.content, 2.0);

    if (strlen(widget->payload.select.data.content))
        alfi_style_filltext(&widget->frame.select.data, widget->payload.select.data.content);

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

static void widget_subheader_keyframe(struct alfi_frame_subheader *keyframe, struct alfi_payload_subheader *subheader, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.subheader.label.content = createstring(widget->payload.subheader.label.content, "");

}

static void widget_subheader_destroy(struct alfi_widget *widget)
{

    widget->payload.subheader.label.content = destroystring(widget->payload.subheader.label.content);

}

static void widget_subheader_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_subheader keyframe;

    widget_subheader_keyframe(&keyframe, &widget->payload.subheader, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.subheader.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.subheader.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->frame.subheader.background.box);

}

static void widget_subheader_render(struct alfi_widget *widget)
{

    if (strlen(widget->payload.subheader.label.content))
        alfi_style_filltext(&widget->frame.subheader.label, widget->payload.subheader.label.content);

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
    float cw = unit_w * widget->payload.table.grid.clength;
    float ch = unit_h * widget->payload.table.grid.rlength;
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

        if (cx >= unit_w * widget->payload.table.grid.csize)
        {

            cx = 0;

            if (widget->payload.table.grid.rlength)
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

static void widget_text_keyframe(struct alfi_frame_text *keyframe, struct alfi_payload_text *text, unsigned int state, float x, float y, float w, float h)
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

    widget->payload.text.label.content = createstring(widget->payload.text.label.content, "");

}

static void widget_text_destroy(struct alfi_widget *widget)
{

    widget->payload.text.label.content = destroystring(widget->payload.text.label.content);

}

static void widget_text_place(struct alfi_widget *widget, float x, float y, float w, float h, float u)
{

    struct alfi_frame_text keyframe;

    widget_text_keyframe(&keyframe, &widget->payload.text, widget->state, x, y, w, h);
    alfi_style_tween(&widget->frame.text.background, &keyframe.background, u);
    alfi_style_tween(&widget->frame.text.label, &keyframe.label, u);
    box_clone(&widget->bb, &widget->frame.text.background.box);

}

static void widget_text_render(struct alfi_widget *widget)
{

    if (strlen(widget->payload.text.label.content))
        alfi_style_filltext(&widget->frame.text.label, widget->payload.text.label.content);

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

static void widget_window_keyframe(struct alfi_frame_window *keyframe, struct alfi_payload_window *window, unsigned int state, float x, float y, float w, float h)
{

    alfi_style_init(&keyframe->background);
    color_clone(&keyframe->background.color, &color_background);
    box_init(&keyframe->background.box, x, y, w, h, 0);

}

static void widget_window_create(struct alfi_widget *widget)
{

    widget->payload.window.label.content = createstring(widget->payload.window.label.content, "undefined");

}

static void widget_window_destroy(struct alfi_widget *widget)
{

    widget->payload.window.label.content = destroystring(widget->payload.window.label.content);

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

    widget_window_keyframe(&keyframe, &widget->payload.window, widget->state, x, y, selfw, selfh);
    alfi_style_tween(&widget->frame.window.background, &keyframe.background, u);
    box_init(&widget->bb, x, y, selfw, selfh, 0);

}

static void widget_window_render(struct alfi_widget *widget)
{

    struct alfi_widget *child = 0;

    alfi_style_fillrect(&widget->frame.window.background);

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

static void onerror(int error, const char *desc)
{

    printf("GLFW error %d: %s\n", error, desc);

}

static void onwindowsize(GLFWwindow *window, int width, int height)
{

    configure(width, height);

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

        case GLFW_KEY_B:
            if (mods & GLFW_MOD_CONTROL)
                loadblank("navi://bookmarks", 0, 0);

            break;

        case GLFW_KEY_D:
            if (mods & GLFW_MOD_CONTROL)
                loadblank("file://", 0, 0);

            break;

        case GLFW_KEY_L:
            if (mods & GLFW_MOD_CONTROL)
                loadblank("navi://address", 0, 0);

            break;

        case GLFW_KEY_M:
            if (mods & GLFW_MOD_CONTROL)
                theme(1);

            break;

        case GLFW_KEY_N:
            if (mods & GLFW_MOD_CONTROL)
                theme(0);

            break;

        case GLFW_KEY_Q:
            if (mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(window, GL_TRUE);

            break;

        case GLFW_KEY_R:
            if (mods & GLFW_MOD_CONTROL)
                loadblank(res_page.url, 0, 0);

            break;

        }

    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch (key)
        {

        case GLFW_KEY_UP:
            scroll(0, 1);

            break;

        case GLFW_KEY_DOWN:
            scroll(0, -1);

            break;

        case GLFW_KEY_LEFT:
            if (focus && focus->header.type == ALFI_WIDGET_FIELD)
            {

                focus->payload.field.data.offset--;

                if (focus->payload.field.data.offset < 0)
                    focus->payload.field.data.offset = 0;

            }

            else
            {

                scroll(1, 0);

            }

            break;

        case GLFW_KEY_RIGHT:
            if (focus && focus->header.type == ALFI_WIDGET_FIELD)
            {

                unsigned int len = strlen(focus->payload.field.data.content);

                focus->payload.field.data.offset++;

                if (focus->payload.field.data.offset > len)
                    focus->payload.field.data.offset = len;

            }

            else
            {

                scroll(-1, 0);

            }

            break;

        case GLFW_KEY_PAGE_UP:
            scroll(0, 16);

            break;

        case GLFW_KEY_PAGE_DOWN:
            scroll(0, -16);

            break;

        case GLFW_KEY_HOME:
            scroll(16, 0);

            break;

        case GLFW_KEY_END:
            scroll(-16, 0);

            break;

        case GLFW_KEY_TAB:
            if (mods & GLFW_MOD_SHIFT)
            {

                struct alfi_widget *widget = prevflag(focus, ALFI_FLAG_FOCUSABLE);

                if (!widget)
                    widget = prevflag(0, ALFI_FLAG_FOCUSABLE);

                setfocus(widget);

            }

            else
            {

                struct alfi_widget *widget = nextflag(focus, ALFI_FLAG_FOCUSABLE);

                if (!widget)
                    widget = nextflag(0, ALFI_FLAG_FOCUSABLE);

                setfocus(widget);

            }

            break;

        case GLFW_KEY_ENTER:
            if (focus)
            {

                if (focus->header.type == ALFI_WIDGET_FIELD)
                {

                    if (focus->payload.field.data.offset < focus->payload.field.data.total - 1)
                    {

                        focus->payload.field.data.content[focus->payload.field.data.offset] = '\n';
                        focus->payload.field.data.offset++;
                        focus->payload.field.data.content[focus->payload.field.data.offset] = '\0';

                    }

                }

            }

            break;

        case GLFW_KEY_BACKSPACE:
            if (focus)
            {

                if (focus->header.type == ALFI_WIDGET_FIELD)
                {

                    if (focus->payload.field.data.offset > 0)
                    {

                        focus->payload.field.data.offset--;
                        focus->payload.field.data.content[focus->payload.field.data.offset] = '\0';

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
            historyindex = 0;

            if (hover)
                call_onclick(hover);
            else
                setfocus(0);

            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            loadblank("navi://address", 0, 0);

            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (mods & GLFW_MOD_SHIFT)
            {

                if (hover)
                {

                    hover = parser_destroywidget(hover);

                    sethover(hover);

                }

            }

            else
            {

                char buffer[32];
                unsigned int count = sprintf(buffer, "item=%d", ++historyindex);

                loadblank("navi://history", buffer, count);

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

    scroll(x, y);

}

static void onchar(GLFWwindow *window, unsigned int codepoint)
{

    if (focus)
    {

        if (focus->header.type == ALFI_WIDGET_FIELD)
        {

            if (focus->payload.field.data.offset < focus->payload.field.data.total - 1)
            {

                focus->payload.field.data.content[focus->payload.field.data.offset] = codepoint;
                focus->payload.field.data.offset++;
                focus->payload.field.data.content[focus->payload.field.data.offset] = '\0';

            }

        }

    }

}

static void setupresources(void)
{

    resource_seturl(&res_font_regular, "/usr/share/navi/roboto-regular.ttf");
    resource_seturl(&res_font_bold, "/usr/share/navi/roboto-bold.ttf");
    resource_seturl(&res_font_icon, "/usr/share/navi/icofont.ttf");

}

static void setupfonts(void)
{

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

static void setupcalls(void)
{

    call_register(ALFI_WIDGET_ANCHOR, ALFI_FLAG_ITEM, widget_anchor_create, widget_anchor_destroy, widget_anchor_place, widget_anchor_render, widget_anchor_setstate, widget_anchor_onclick, widget_anchor_getcursor);
    call_register(ALFI_WIDGET_BUTTON, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_button_create, widget_button_destroy, widget_button_place, widget_button_render, widget_button_setstate, widget_button_onclick, widget_button_getcursor);
    call_register(ALFI_WIDGET_CHOICE, ALFI_FLAG_CONTAINER, widget_choice_create, widget_choice_destroy, widget_choice_place, widget_choice_render, widget_choice_setstate, widget_choice_onclick, widget_choice_getcursor);
    call_register(ALFI_WIDGET_DIVIDER, ALFI_FLAG_ITEM, widget_divider_create, widget_divider_destroy, widget_divider_place, widget_divider_render, widget_divider_setstate, widget_divider_onclick, widget_divider_getcursor);
    call_register(ALFI_WIDGET_HSTACK, ALFI_FLAG_CONTAINER, 0, 0, widget_hstack_place, widget_hstack_render, widget_hstack_setstate, widget_hstack_onclick, widget_hstack_getcursor);
    call_register(ALFI_WIDGET_FIELD, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_field_create, widget_field_destroy, widget_field_place, widget_field_render, widget_field_setstate, widget_field_onclick, widget_field_getcursor);
    call_register(ALFI_WIDGET_HEADER, ALFI_FLAG_ITEM, widget_header_create, widget_header_destroy, widget_header_place, widget_header_render, widget_header_setstate, widget_header_onclick, widget_header_getcursor);
    call_register(ALFI_WIDGET_IMAGE, ALFI_FLAG_ITEM, 0, 0, widget_image_place, widget_image_render, widget_image_setstate, widget_image_onclick, widget_image_getcursor);
    call_register(ALFI_WIDGET_LIST, ALFI_FLAG_CONTAINER, 0, 0, widget_list_place, widget_list_render, widget_list_setstate, widget_list_onclick, widget_list_getcursor);
    call_register(ALFI_WIDGET_SELECT, ALFI_FLAG_ITEM | ALFI_FLAG_FOCUSABLE, widget_select_create, widget_select_destroy, widget_select_place, widget_select_render, widget_select_setstate, widget_select_onclick, widget_select_getcursor);
    call_register(ALFI_WIDGET_SUBHEADER, ALFI_FLAG_ITEM, widget_subheader_create, widget_subheader_destroy, widget_subheader_place, widget_subheader_render, widget_subheader_setstate, widget_subheader_onclick, widget_subheader_getcursor);
    call_register(ALFI_WIDGET_TABLE, ALFI_FLAG_CONTAINER, 0, 0, widget_table_place, widget_table_render, widget_table_setstate, widget_table_onclick, widget_table_getcursor);
    call_register(ALFI_WIDGET_TEXT, ALFI_FLAG_ITEM, widget_text_create, widget_text_destroy, widget_text_place, widget_text_render, widget_text_setstate, widget_text_onclick, widget_text_getcursor);
    call_register(ALFI_WIDGET_VSTACK, ALFI_FLAG_CONTAINER, 0, 0, widget_vstack_place, widget_vstack_render, widget_vstack_setstate, widget_vstack_onclick, widget_vstack_getcursor);
    call_register(ALFI_WIDGET_WINDOW, ALFI_FLAG_CONTAINER, widget_window_create, widget_window_destroy, widget_window_place, widget_window_render, widget_window_setstate, widget_window_onclick, widget_window_getcursor);

}

static void renderbackground(void)
{

    struct alfi_style background;

    box_init(&background.box, 0, 0, page_w, page_h, 0);
    color_clone(&background.color, &color_background);
    alfi_style_fillrect(&background);

}

static void render(GLFWwindow *window)
{

    struct alfi_widget *root = pool_findbyname("window");

    nvg_reset(&ctx);
    nvg_gl_reset(&glctx, page_w, page_h);
    renderbackground();

    if (root)
    {

        adjust(root->bb.w, root->bb.h);
        call_place(root, scroll_x, scroll_y, 0, 0, 0.5);
        call_render(root);

    }

    nvg_gl_flush(&glctx);
    glfwSwapBuffers(window);

}

static struct alfi_widget *findtouchingwidget(struct alfi_widget *widget, float x, float y)
{

    struct alfi_widget *child = 0;

    while ((child = pool_nextchild(child, widget)))
    {

        if (box_istouching(&child->bb, x, y))
            return findtouchingwidget(child, x, y);

    }

    return widget;

}

static void precheck(GLFWwindow *window)
{

    struct alfi_widget *root = pool_findbyname("window");
    struct alfi_widget *main = pool_findbyname("main");
    struct alfi_widget *touch = findtouchingwidget(main, mouse_x, mouse_y);

    sethover(touch);

    if (touch)
        setcursor(window, call_getcursor(touch, mouse_x, mouse_y));

    if (updatetitle)
    {

        glfwSetWindowTitle(window, root->payload.window.label.content);

        updatetitle = 0;

    }

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
        precheck(window);

        if (frames)
            render(window);

    }

}

int main(int argc, char **argv)
{

    const GLFWvidmode *mode;
    GLFWmonitor *monitor;
    GLFWwindow *window;

    glfwInit();
    glfwSetErrorCallback(onerror);

    monitor = glfwGetPrimaryMonitor();
    mode = glfwGetVideoMode(monitor);
    window = glfwCreateWindow(mode->width, mode->height, "Navi 0.1", 0, 0);

    configure(mode->width, mode->height);
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
    parser_init(&parser, parser_fail, pool_findbyname, parser_createwidget, parser_destroywidget, pool_allocate);
    pool_setup();
    setupcalls();
    setupresources();
    setupcursors();
    setupfonts();
    theme(0);
    loadbase();
    render(window);
    loadblank("navi://address", 0, 0);
    run(window);
    nvg_gl_destroy(&glctx);
    fons_delete(&fsctx);
    glfwTerminate();

    return 0;

}

