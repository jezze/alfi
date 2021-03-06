#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#if defined NVG_GL_GLEW
#include <GL/glew.h>
#endif

#if defined NVG_GL_VERSION_GLES2
#include <GLES2/gl2.h>
#endif

#if defined NVG_GL_VERSION_GLES3
#include <GLES3/gl3.h>
#endif

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
#include "attribute.h"
#include "widget.h"
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
    float width;

};

static unsigned int getchartype(unsigned int codepoint)
{

    switch (codepoint)
    {

    case 9:
    case 11:
    case 12:
    case 32:
    case 0x00a0:
        return CHARTYPE_SPACE;

    case 10:
    case 0x0085:
        return CHARTYPE_NEWLINE;

    default:
        return CHARTYPE_CHAR;

    }

    return 0;

}

static const char *calcline(struct style_font *font, float width, const char *string, const char *end, struct textrow *row)
{

    struct fons_textiter iter;
    float rowWidth = 0;
    const char *rowStart = 0;
    const char *rowEnd = 0;
    const char *wordStart = 0;
    const char *breakEnd = 0;
    float breakWidth = 0;
    unsigned int ptype = CHARTYPE_SPACE;

    fons_inititer(&fsctx, &iter, &fsctx.fonts[font->face], font->align, font->size, 0, 0, string, end);

    while (fons_nextiter(&fsctx, &iter))
    {

        unsigned int type = getchartype(iter.codepoint);

        switch (type)
        {

        case CHARTYPE_NEWLINE:
            row->start = rowStart != 0 ? rowStart : iter.str;
            row->end = rowEnd != 0 ? rowEnd : iter.str;
            row->width = rowWidth;

            return iter.next;

        case CHARTYPE_SPACE:
            if (ptype == CHARTYPE_CHAR)
            {

                breakEnd = iter.str;
                breakWidth = rowWidth;

            }

            break;

        case CHARTYPE_CHAR:
            if (!rowStart)
            {

                rowStart = iter.str;
                wordStart = iter.str;
                breakEnd = rowStart;
                breakWidth = 0.0;

            }

            rowEnd = iter.next;
            rowWidth = iter.nextx;

            if (ptype == CHARTYPE_SPACE)
            {

                wordStart = iter.str;

            }

            if (iter.nextx > width)
            {

                if (breakEnd == rowStart)
                {

                    row->start = rowStart;
                    row->end = iter.str;
                    row->width = rowWidth;

                    return iter.str;

                }

                else
                {

                    row->start = rowStart;
                    row->end = breakEnd;
                    row->width = breakWidth;

                    return wordStart;

                }

            }

            break;

        }

        ptype = type;

    }

    if (rowStart)
    {

        row->start = rowStart;
        row->end = rowEnd;
        row->width = rowWidth;

        return end;

    }

    return 0;

}

static void renderfill(struct nvg_paint *paint, struct nvg_scissor *scissor)
{

    nvg_flatten(&ctx);
    nvg_expand(&ctx);
    nvg_gl_render_paths(&glctx, paint, scissor, ctx.bounds, ctx.paths, ctx.npaths);

}

static void rendericon(struct nvg_paint *paint, struct nvg_scissor *scissor, struct style_font *font, float x, float y, int codepoint)
{

    struct fons_glyph *glyph = fons_getglyph(&fsctx, &fsctx.fonts[font->face], codepoint, font->size);
    unsigned int nverts = 0;

    if (glyph)
    {

        struct fons_quad q;
        float c[8];

        fons_getquad(&fsctx, glyph, &q, x, y);
        nvg_getpoints(&c[0], &c[1], ctx.xform, q.x0, q.y0);
        nvg_getpoints(&c[2], &c[3], ctx.xform, q.x1, q.y0);
        nvg_getpoints(&c[4], &c[5], ctx.xform, q.x1, q.y1);
        nvg_getpoints(&c[6], &c[7], ctx.xform, q.x0, q.y1);

        if (nverts + 6 <= NVG_VERTSSIZE)
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

    nvg_gl_texture_update(&glctx, glctx.fontimage, 0, 0, fsctx.width, fsctx.height, fsctx.texdata);

    paint->image = glctx.fontimage;

    nvg_gl_render_vertices(&glctx, paint, scissor, ctx.verts, nverts);

}

static float rendertext(struct nvg_paint *paint, struct nvg_scissor *scissor, struct style_font *font, float x, float y, const char *string, const char *end)
{

    struct fons_textiter iter;
    unsigned int nverts = 0;

    fons_inititer(&fsctx, &iter, &fsctx.fonts[font->face], font->align, font->size, x, y, string, end);

    while (fons_nextiter(&fsctx, &iter))
    {

        struct fons_glyph *glyph = fons_getglyph(&fsctx, iter.font, iter.codepoint, iter.size);

        if (glyph)
        {

            struct fons_quad q;
            float c[8];

            fons_getquad(&fsctx, glyph, &q, iter.x, iter.y);
            nvg_getpoints(&c[0], &c[1], ctx.xform, q.x0, q.y0);
            nvg_getpoints(&c[2], &c[3], ctx.xform, q.x1, q.y0);
            nvg_getpoints(&c[4], &c[5], ctx.xform, q.x1, q.y1);
            nvg_getpoints(&c[6], &c[7], ctx.xform, q.x0, q.y1);

            if (nverts + 6 <= NVG_VERTSSIZE)
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

    }

    nvg_gl_texture_update(&glctx, glctx.fontimage, 0, 0, fsctx.width, fsctx.height, fsctx.texdata);

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

    }

    return resource;

}

struct resource *render_loadfont(char *url)
{

    struct resource *resource = getresource(url);

    if (!resource)
        return 0;

    resource_iref(resource);

    if (resource->index)
        return resource;

    resource_load(resource, 0, 0);

    resource->index = fons_addfont(&fsctx, resource->data, resource->count);

    return resource;

}

struct resource *render_loadimage(char *url)
{

    struct resource *resource = getresource(url);
    unsigned char *data;

    if (!resource)
        return 0;

    resource_iref(resource);

    if (resource->index)
        return resource;

    resource_load(resource, 0, 0);

    data = stbi_load_from_memory(resource->data, resource->count, &resource->w, &resource->h, &resource->n, 4);

    if (data)
    {

        resource->index = nvg_gl_texture_create(&glctx, NVG_TEXTURE_RGBA, resource->w, resource->h, 0, data);

        stbi_image_free(data);

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

    fons_create(&fsctx, 512, 512);
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
    const char *end = current + strlen(text) + 1;
    float w = 0;

    while ((current = calcline(&style->font, style->box.w, current, end, &row)))
    {

        if (w < row.width)
            w = row.width;

    }

    return w + 2;

}

float render_textheight(struct style *style, char *text)
{

    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text) + 1;
    unsigned int i;

    for (i = 0; (current = calcline(&style->font, style->box.w, current, end, &row)); i++);

    return i * style->font.size + 2;

}

void render_filltext(struct style *style, char *text)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;
    struct textrow row;
    const char *current = text;
    const char *end = current + strlen(text) + 1;
    float x = style->box.x;
    float y = style->box.y;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, style->color.r, style->color.g, style->color.b, style->color.a);

    while ((current = calcline(&style->font, style->box.w, current, end, &row)))
    {

        if (style->font.align & STYLE_ALIGN_RIGHT)
            rendertext(&paint, &scissor, &style->font, x + style->box.w, y, row.start, row.end);
        else if (style->font.align & STYLE_ALIGN_CENTER)
            rendertext(&paint, &scissor, &style->font, x + style->box.w / 2, y, row.start, row.end);
        else
            rendertext(&paint, &scissor, &style->font, x, y, row.start, row.end);

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
    const char *end = current + strlen(text) + 1;
    const char *last = text;
    float x = style->box.x;
    float y = style->box.y;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&textpaint, style->color.r, style->color.g, style->color.b, style->color.a);
    nvg_paint_color(&cursorpaint, frame_color_focus->r, frame_color_focus->g, frame_color_focus->b, frame_color_focus->a);

    while ((current = calcline(&style->font, style->box.w, current, end, &row)))
    {

        if (offset >= 0 && offset <= row.end - row.start)
        {

            const char *split = row.start + offset;

            x = rendertext(&textpaint, &scissor, &style->font, x, y, row.start, split);

            rendertext(&textpaint, &scissor, &style->font, x, y, split, row.end);
            nvg_path_begin(&ctx);
            nvg_path_roundedrect(&ctx, x, y, 3, style->font.size, 0);
            renderfill(&cursorpaint, &scissor);

            x = style->box.x;

        }

        else
        {

            rendertext(&textpaint, &scissor, &style->font, x, y, row.start, row.end);

        }

        offset -= (int)(current - last);
        last = current;
        y += style->font.size;

    }

}

void render_fillicon(struct style *style, int type)
{

    struct nvg_scissor scissor;
    struct nvg_paint paint;

    nvg_scissor_init(&scissor);
    nvg_paint_color(&paint, style->color.r, style->color.g, style->color.b, style->color.a);
    rendericon(&paint, &scissor, &style->font, style->box.x, style->box.y, type);

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

