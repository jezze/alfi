#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <GL/glew.h>
#include "stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "fons.h"
#include "nvg.h"
#include "nvg_gl.h"
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "widgets.h"
#include "parser.h"
#include "render.h"
#include "pool.h"

#define CHARTYPE_NONE                   0
#define CHARTYPE_SPACE                  1
#define CHARTYPE_NEWLINE                2
#define CHARTYPE_CHAR                   3

static struct nvg_context ctx;
static struct nvg_gl_context glctx;
static struct fons_context fsctx;

struct textrow
{

    const char *start;
    const char *end;
    const char *next;
    float width;

};

static const char *calcline(struct style_font *font, float width, const char *string, const char *end, struct textrow *row)
{

    struct fons_textiter iter;
    struct fons_quad q;
    float rowWidth = 0;
    const char *rowStart = 0;
    const char *rowEnd = 0;
    const char *wordStart = 0;
    const char *breakEnd = 0;
    float breakWidth = 0;
    int type = CHARTYPE_SPACE;
    int ptype = CHARTYPE_SPACE;

    fons_inititer(&fsctx, &iter, font->face, font->align, font->size, 0, 0, 0, string, end, FONS_GLYPH_BITMAP_OPTIONAL);

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
            case 0x0085:
                type = CHARTYPE_NEWLINE;

                break;

            default:
                type = CHARTYPE_CHAR;

                break;

        }

        if (type == CHARTYPE_NEWLINE)
        {

            row->start = rowStart != 0 ? rowStart : iter.str;
            row->end = rowEnd != 0 ? rowEnd : iter.str;
            row->width = rowWidth;
            row->next = iter.next;

            return row->next;

        }

        else
        {

            if (rowStart == 0)
            {

                if (type == CHARTYPE_CHAR)
                {

                    rowStart = iter.str;
                    rowEnd = iter.next;
                    rowWidth = iter.nextx;
                    wordStart = iter.str;
                    breakEnd = rowStart;
                    breakWidth = 0.0;

                }

            }

            else
            {

                float nextWidth = iter.nextx;

                if (type == CHARTYPE_CHAR)
                {

                    rowEnd = iter.next;
                    rowWidth = iter.nextx;

                }

                if (type == CHARTYPE_SPACE && ptype == CHARTYPE_CHAR)
                {

                    breakEnd = iter.str;
                    breakWidth = rowWidth;

                }

                if (type == CHARTYPE_CHAR && ptype == CHARTYPE_SPACE)
                {

                    wordStart = iter.str;

                }

                if (type == CHARTYPE_CHAR && nextWidth > width)
                {

                    if (breakEnd == rowStart)
                    {

                        row->start = rowStart;
                        row->end = iter.str;
                        row->width = rowWidth;
                        row->next = iter.str;

                        return row->next;

                    }

                    else
                    {

                        row->start = rowStart;
                        row->end = breakEnd;
                        row->width = breakWidth;
                        row->next = wordStart;

                        return row->next;

                    }

                }

            }

        }

        ptype = type;

    }

    if (rowStart != 0)
    {

        row->start = rowStart;
        row->end = rowEnd;
        row->width = rowWidth;
        row->next = end;

        return row->next;

    }

    return 0;

}

static void renderfill(struct nvg_paint *paint, struct nvg_scissor *scissor)
{

    nvg_flatten(&ctx);
    nvg_expand(&ctx);
    nvg_gl_render_paths(&glctx, paint, scissor, ctx.bounds, ctx.paths, ctx.npaths);

}

static float rendertext(struct nvg_paint *paint, struct nvg_scissor *scissor, int font, int align, float size, float x, float y, const char *string, const char *end)
{

    struct fons_textiter iter;
    struct fons_quad q;
    int cverts = (end - string) * 6;
    int nverts = 0;
    int dirty[4];

    fons_inititer(&fsctx, &iter, font, align, size, 0, x, y, string, end, FONS_GLYPH_BITMAP_REQUIRED);

    while (fons_nextiter(&fsctx, &iter, &q))
    {

        float c[8];

        nvg_getpoints(&c[0], &c[1], ctx.xform, q.x0, q.y0);
        nvg_getpoints(&c[2], &c[3], ctx.xform, q.x1, q.y0);
        nvg_getpoints(&c[4], &c[5], ctx.xform, q.x1, q.y1);
        nvg_getpoints(&c[6], &c[7], ctx.xform, q.x0, q.y1);

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

static struct resource *getresource(char *url)
{

    struct resource *resource = pool_resource_find(url);

    if (!resource)
    {

        resource = pool_resource_create();

        if (!resource)
            return 0;

        resource_init(resource, url);
        resource_load(resource, 0, 0);

    }

    return resource;

}

struct resource *render_loadfont(char *url)
{

    struct resource *resource = getresource(url);

    if (!resource)
        return 0;

    if (resource->index)
        return resource;

    resource_iref(resource);

    resource->index = fons_addfont(&fsctx, resource->data, resource->count);

    return resource;

}

struct resource *render_loadimage(char *url)
{

    struct resource *resource = getresource(url);

    if (!resource)
        return 0;

    if (resource->index)
        return resource;

    resource_iref(resource);

    resource->img = stbi_load_from_memory(resource->data, resource->count, &resource->w, &resource->h, &resource->n, 4);

    if (resource->img)
    {

        resource->index = nvg_gl_texture_create(&glctx, NVG_TEXTURE_RGBA, resource->w, resource->h, 0, resource->img);

        stbi_image_free(resource->img);

    }

    return resource;

}

void render_unloadimage(struct resource *resource)
{

    nvg_gl_texture_destroy(&glctx, resource->index);

    resource_dref(resource);

}

void render_reset(float w, float h)
{

    nvg_reset(&ctx);
    nvg_gl_reset(&glctx, w, h);

}

void render_flush(void)
{

    nvg_gl_flush(&glctx);

}

void render_create(void)
{

    fons_create(&fsctx, 512, 512, FONS_ZERO_TOPLEFT);
    nvg_gl_create(&glctx, fsctx.width, fsctx.height);

}

void render_destroy(void)
{

    fons_delete(&fsctx);
    nvg_gl_destroy(&glctx);

}

float render_textwidth(struct style *style, char *text)
{

    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float w = 0;

    while ((current = calcline(&style->font, style->box.w, current, end, &row)))
    {

        if (w < row.width)
            w = row.width;

    }

    return w;

}

float render_textheight(struct style *style, char *text)
{

    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    unsigned int i;

    for (i = 0; (current = calcline(&style->font, style->box.w, current, end, &row)); i++);

    return (i) ? i * style->font.size : style->font.size;

}

void render_filltext(struct style *style, char *text)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;
    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float x = style->box.x;
    float y = style->box.y;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, style->color.r, style->color.g, style->color.b, style->color.a);

    while ((current = calcline(&style->font, style->box.w, current, end, &row)))
    {

        rendertext(&paint, &scissor, style->font.face, style->font.align, style->font.size, x, y, row.start, row.end);

        y += style->font.size;

    }

}

void render_filltextinput(struct style *style, char *text, int offset, struct style_color *frame_color_focus)
{

    struct nvg_scissor scissor;
    struct nvg_paint textpaint;
    struct nvg_paint cursorpaint;
    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text);
    float x = style->box.x;
    float y = style->box.y;
    unsigned int i;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&textpaint, style->color.r, style->color.g, style->color.b, style->color.a);
    nvg_paint_color(&cursorpaint, frame_color_focus->r, frame_color_focus->g, frame_color_focus->b, frame_color_focus->a);

    for (i = 0; (current = calcline(&style->font, style->box.w, current, end, &row)); i++)
    {

        int length = row.end - row.start;

        if (offset >= 0 && offset <= length)
        {

            x = rendertext(&textpaint, &scissor, style->font.face, style->font.align, style->font.size, x, y, row.start, row.start + offset);

            rendertext(&textpaint, &scissor, style->font.face, style->font.align, style->font.size, x, y, row.start + offset, row.end);
            nvg_path_begin(&ctx);
            nvg_path_roundedrect(&ctx, x, y, 3, style->font.size, 0);
            renderfill(&cursorpaint, &scissor);

            x = style->box.x;

        }

        else
        {

            rendertext(&textpaint, &scissor, style->font.face, style->font.align, style->font.size, x, y, row.start, row.end);

        }

        offset -= length;
        y += style->font.size;

    }

}

void render_fillrectborder(struct style *border, float bordersize)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, border->color.r, border->color.g, border->color.b, border->color.a);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h, border->box.r);
    nvg_path_hole(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x + bordersize, border->box.y + bordersize, border->box.w - bordersize * 2, border->box.h - bordersize * 2, border->box.r);
    renderfill(&paint, &scissor);

}

void render_fillrectbordergap(struct style *border, float bordersize, float x, float w)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, border->color.r, border->color.g, border->color.b, border->color.a);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h, border->box.r);
    nvg_path_hole(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x + bordersize, border->box.y + bordersize, border->box.w - bordersize * 2, border->box.h - bordersize * 2, border->box.r);
    nvg_path_rect(&ctx, x, border->box.y, w, bordersize);
    renderfill(&paint, &scissor);

}

void render_fillrect(struct style *style)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, style->color.r, style->color.g, style->color.b, style->color.a);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, style->box.x, style->box.y, style->box.w, style->box.h, style->box.r);
    renderfill(&paint, &scissor);

}

void render_fillrectgap(struct style *border, float x, float w)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, border->color.r, border->color.g, border->color.b, border->color.a);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, border->box.x, border->box.y, border->box.w, border->box.h, border->box.r);
    nvg_path_hole(&ctx);
    nvg_path_rect(&ctx, x, border->box.y, w, border->box.h);
    renderfill(&paint, &scissor);

}

void render_fillcircle(struct style *style)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, style->color.r, style->color.g, style->color.b, style->color.a);
    nvg_path_begin(&ctx);
    nvg_path_circle(&ctx, style->box.x, style->box.y, style->box.r);
    renderfill(&paint, &scissor);

}

void render_fillimage(struct style *style, int image)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;

    nvg_scissor_init(&scissor);
    nvg_paint_image(&paint, style->box.x, style->box.y, style->box.w, style->box.h, 0.0, image, 1.0);
    nvg_path_begin(&ctx);
    nvg_path_roundedrect(&ctx, style->box.x, style->box.y, style->box.w, style->box.h, style->box.r);
    renderfill(&paint, &scissor);

}

void render_background(int w, int h, struct style_color *color)
{

    struct style background;

    style_box_init(&background.box, 0, 0, w, h, 0);
    style_color_clone(&background.color, color);
    render_fillrect(&background);

}

