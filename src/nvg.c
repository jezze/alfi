#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "nvg.h"

#define NVG_PI 3.14159265358979323846264338327f
#define NVG_KAPPA90 0.5522847493f
#define NVG_COUNTOF(arr) (sizeof (arr) / sizeof (0[arr]))
#define NVG_DISTTOL 0.01f
#define NVG_TESSTOL 0.25f

enum nvg_commands
{

    NVG_MOVETO = 0,
    NVG_LINETO = 1,
    NVG_BEZIERTO = 2,
    NVG_CLOSE = 3,
    NVG_WINDING = 4

};

enum nvg_winding
{

    NVG_CCW = 1,
    NVG_CW = 2

};

static float minf(float a, float b)
{

    return a < b ? a : b;

}

static float maxf(float a, float b)
{

    return a > b ? a : b;

}

static float absf(float a)
{

    return a >= 0.0f ? a : -a;

}

static float signf(float a)
{

    return a >= 0.0f ? 1.0f : -1.0f;

}

static float normalize(float *x, float *y)
{

    float d = sqrtf((*x) * (*x) + (*y) * (*y));

    if (d > 1e-6f)
    {

        float id = 1.0f / d;

        *x *= id;
        *y *= id;

    }

    return d;

}

struct nvg_color nvg_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{

    struct nvg_color color;

    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = a / 255.0f;

    return color;

}

struct nvg_color nvg_rgbaf(float r, float g, float b, float a)
{

    struct nvg_color color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;

}

struct nvg_color nvg_premulrgba(struct nvg_color c)
{

    c.r *= c.a;
    c.g *= c.a;
    c.b *= c.a;

    return c;

}

void nvg_paint_color(struct nvg_paint *paint, float r, float g, float b, float a)
{

    memset(paint, 0, sizeof (struct nvg_paint));
    nvg_xform_identity(paint->xform);

    paint->radius = 0.0f;
    paint->feather = 1.0f;
    paint->innerColor = nvg_rgba(r, g, b, a);
    paint->outerColor = nvg_rgba(r, g, b, a);

}

void nvg_paint_image(struct nvg_paint *paint, float cx, float cy, float w, float h, float angle, int image, float alpha)
{

    memset(paint, 0, sizeof (struct nvg_paint));
    nvg_xform_rotate(paint->xform, angle);

    paint->xform[4] = cx;
    paint->xform[5] = cy;
    paint->extent[0] = w;
    paint->extent[1] = h;
    paint->image = image;
    paint->innerColor = nvg_rgbaf(1, 1, 1, alpha);
    paint->outerColor = nvg_rgbaf(1, 1, 1, alpha);

}

void nvg_paint_boxgradient(struct nvg_paint *paint, float x, float y, float w, float h, float r, float f, struct nvg_color icol, struct nvg_color ocol)
{

    memset(paint, 0, sizeof (struct nvg_paint));
    nvg_xform_identity(paint->xform);

    paint->xform[4] = x + w * 0.5f;
    paint->xform[5] = y + h * 0.5f;
    paint->extent[0] = w * 0.5f;
    paint->extent[1] = h * 0.5f;
    paint->radius = r;
    paint->feather = maxf(1.0f, f);
    paint->innerColor = icol;
    paint->outerColor = ocol;

}

void nvg_paint_lineargradient(struct nvg_paint *paint, float sx, float sy, float ex, float ey, struct nvg_color icol, struct nvg_color ocol)
{

    float dx = ex - sx;
    float dy = ey - sy;
    float d = sqrtf(dx * dx + dy * dy);
    const float large = 1e5;

    dx /= d;
    dy /= d;

    memset(paint, 0, sizeof (struct nvg_paint));

    paint->xform[0] = dy;
    paint->xform[1] = -dx;
    paint->xform[2] = dx;
    paint->xform[3] = dy;
    paint->xform[4] = sx - dx * large;
    paint->xform[5] = sy - dy * large;
    paint->extent[0] = large;
    paint->extent[1] = large + d * 0.5f;
    paint->radius = 0.0f;
    paint->feather = maxf(1.0f, d);
    paint->innerColor = icol;
    paint->outerColor = ocol;

}

void nvg_paint_radialgradient(struct nvg_paint *paint, float cx, float cy, float inr, float outr, struct nvg_color icol, struct nvg_color ocol)
{

    float r = (inr + outr) * 0.5f;
    float f = (outr - inr);

    memset(paint, 0, sizeof (struct nvg_paint));
    nvg_xform_identity(paint->xform);

    paint->xform[4] = cx;
    paint->xform[5] = cy;
    paint->extent[0] = r;
    paint->extent[1] = r;
    paint->radius = r;
    paint->feather = maxf(1.0f, f);
    paint->innerColor = icol;
    paint->outerColor = ocol;

}

void nvg_setvertex(struct nvg_vertex *vtx, float x, float y, float u, float v)
{

    vtx->x = x;
    vtx->y = y;
    vtx->u = u;
    vtx->v = v;

}

void nvg_xform_identity(float *t)
{

    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvg_xform_translate(float *t, float tx, float ty)
{

    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = tx;
    t[5] = ty;

}

void nvg_xform_scale(float *t, float sx, float sy)
{

    t[0] = sx;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = sy;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvg_xform_rotate(float *t, float a)
{

    float cs = cosf(a);
    float sn = sinf(a);

    t[0] = cs;
    t[1] = sn;
    t[2] = -sn;
    t[3] = cs;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvg_xform_skewx(float *t, float a)
{

    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = tanf(a);
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvg_xform_skewy(float *t, float a)
{

    t[0] = 1.0f;
    t[1] = tanf(a);
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvg_xform_multiply(float *t, const float *s)
{

    float t0 = t[0] * s[0] + t[1] * s[2];
    float t2 = t[2] * s[0] + t[3] * s[2];
    float t4 = t[4] * s[0] + t[5] * s[2] + s[4];

    t[1] = t[0] * s[1] + t[1] * s[3];
    t[3] = t[2] * s[1] + t[3] * s[3];
    t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
    t[0] = t0;
    t[2] = t2;
    t[4] = t4;

}

void nvg_xform_premultiply(float *t, const float *s)
{

    float s2[6];

    memcpy(s2, s, sizeof (float) * 6);
    nvg_xform_multiply(s2, t);
    memcpy(t, s2, sizeof (float) * 6);

}

void nvg_xform_inverse(float *t, const float *s)
{

    double det = (double)s[0] * s[3] - (double)s[2] * s[1];
    double invdet = 1.0 / det;

    if (det > -1e-6 && det < 1e-6)
    {

        nvg_xform_identity(t);

        return;

    }

    t[0] = (float)(s[3] * invdet);
    t[2] = (float)(-s[2] * invdet);
    t[4] = (float)(((double)s[2] * s[5] - (double)s[3] * s[4]) * invdet);
    t[1] = (float)(-s[1] * invdet);
    t[3] = (float)(s[0] * invdet);
    t[5] = (float)(((double)s[1] * s[4] - (double)s[0] * s[5]) * invdet);

}

void nvg_getpoints(float *dx, float *dy, const float *t, float sx, float sy)
{

    *dx = sx * t[0] + sy * t[2] + t[4];
    *dy = sx * t[1] + sy * t[3] + t[5];

}

void nvg_transform(struct nvg_context *ctx, float a, float b, float c, float d, float e, float f)
{

    float t[6] = { a, b, c, d, e, f };

    nvg_xform_premultiply(ctx->xform, t);

}

void nvg_translate(struct nvg_context *ctx, float x, float y)
{

    float t[6];

    nvg_xform_translate(t, x, y);
    nvg_xform_premultiply(ctx->xform, t);

}

void nvg_rotate(struct nvg_context *ctx, float angle)
{

    float t[6];

    nvg_xform_rotate(t, angle);
    nvg_xform_premultiply(ctx->xform, t);

}

void nvg_skewx(struct nvg_context *ctx, float angle)
{

    float t[6];

    nvg_xform_skewx(t, angle);
    nvg_xform_premultiply(ctx->xform, t);

}

void nvg_skewy(struct nvg_context *ctx, float angle)
{

    float t[6];

    nvg_xform_skewy(t, angle);
    nvg_xform_premultiply(ctx->xform, t);

}

void nvg_scale(struct nvg_context *ctx, float x, float y)
{

    float t[6];

    nvg_xform_scale(t, x, y);
    nvg_xform_premultiply(ctx->xform, t);

}

void nvg_scissor_init(struct nvg_scissor *scissor)
{

    memset(scissor->xform, 0, sizeof (scissor->xform));

    scissor->extent[0] = -1.0f;
    scissor->extent[1] = -1.0f;

}

void nvg_scissor_set(struct nvg_scissor *scissor, float *xform, float x, float y, float w, float h)
{

    w = maxf(0.0f, w);
    h = maxf(0.0f, h);

    nvg_xform_identity(scissor->xform);

    scissor->xform[4] = x + w * 0.5f;
    scissor->xform[5] = y + h * 0.5f;

    nvg_xform_multiply(scissor->xform, xform);

    scissor->extent[0] = w * 0.5f;
    scissor->extent[1] = h * 0.5f;

}

static int pointequals(float x1, float y1, float x2, float y2, float tol)
{

    float dx = x2 - x1;
    float dy = y2 - y1;

    return dx * dx + dy * dy < tol * tol;

}

static void appendcommand(struct nvg_context *ctx, float *vals, int nvals)
{

    unsigned int i;

    for (i = 0; i < nvals;)
    {

        int cmd = (int)vals[i];

        switch (cmd)
        {

        case NVG_MOVETO:
            nvg_getpoints(&vals[i + 1], &vals[i + 2], ctx->xform, vals[i + 1], vals[i + 2]);

            i += 3;

            break;

        case NVG_LINETO:
            nvg_getpoints(&vals[i + 1], &vals[i + 2], ctx->xform, vals[i + 1],vals[i + 2]);

            i += 3;

            break;

        case NVG_BEZIERTO:
            nvg_getpoints(&vals[i + 1],&vals[i + 2], ctx->xform, vals[i + 1], vals[i + 2]);
            nvg_getpoints(&vals[i + 3],&vals[i + 4], ctx->xform, vals[i + 3], vals[i + 4]);
            nvg_getpoints(&vals[i + 5],&vals[i + 6], ctx->xform, vals[i + 5], vals[i + 6]);

            i += 7;

            break;

        case NVG_CLOSE:
            i++;

            break;

        case NVG_WINDING:
            i += 2;

            break;

        default:
            i++;

        }

    }

    memcpy(&ctx->commands[ctx->ncommands], vals, nvals * sizeof (float));

    ctx->ncommands += nvals;

}

static struct nvg_path *lastpath(struct nvg_context *ctx)
{

    if (ctx->npaths)
        return &ctx->paths[ctx->npaths - 1];

    return 0;

}

static void addpath(struct nvg_context *ctx)
{

    struct nvg_path *path = &ctx->paths[ctx->npaths];

    memset(path, 0, sizeof (struct nvg_path));

    path->first = ctx->npoints;
    path->winding = NVG_CCW;
    ctx->npaths++;

}

static struct nvg_point *lastpoint(struct nvg_context *ctx)
{

    if (ctx->npoints)
        return &ctx->points[ctx->npoints - 1];

    return 0;

}

static void addpoint(struct nvg_context *ctx, float x, float y)
{

    struct nvg_path *path = lastpath(ctx);
    struct nvg_point *pt;

    if (!path)
        return;

    if (path->count > 0 && ctx->npoints > 0)
    {

        pt = lastpoint(ctx);

        if (pointequals(pt->x, pt->y, x, y, NVG_DISTTOL))
            return;

    }

    pt = &ctx->points[ctx->npoints];

    memset(pt, 0, sizeof (struct nvg_point));

    pt->x = x;
    pt->y = y;
    ctx->npoints++;
    path->count++;

}

static void closepath(struct nvg_context *ctx)
{

    struct nvg_path *path = lastpath(ctx);

    if (!path)
        return;

    path->closed = 1;

}

static void pathwinding(struct nvg_context *ctx, int winding)
{

    struct nvg_path *path = lastpath(ctx);

    if (!path)
        return;

    path->winding = winding;

}

static float triarea2(float ax, float ay, float bx, float by, float cx, float cy)
{

    float abx = bx - ax;
    float aby = by - ay;
    float acx = cx - ax;
    float acy = cy - ay;

    return acx * aby - abx * acy;

}

static float polyarea(struct nvg_point *pts, int npts)
{

    float area = 0;
    unsigned int i;

    for (i = 2; i < npts; i++)
    {

        struct nvg_point *a = &pts[0];
        struct nvg_point *b = &pts[i - 1];
        struct nvg_point *c = &pts[i];

        area += triarea2(a->x, a->y, b->x, b->y, c->x, c->y);

    }

    return area * 0.5f;

}

static void polyreverse(struct nvg_point *pts, int npts)
{

    struct nvg_point tmp;
    unsigned int j = npts - 1;
    unsigned int i;

    for (i = 0; i < j; i++)
    {

        tmp = pts[i];
        pts[i] = pts[j];
        pts[j] = tmp;
        j--;

    }

}

static void tesselatebezier(struct nvg_context *ctx, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int level)
{

    float x12, y12, x23, y23, x34, y34, x123, y123, x234, y234, x1234, y1234;
    float dx, dy, d2, d3;

    if (level > 10)
        return;

    x12 = (x1 + x2) * 0.5f;
    y12 = (y1 + y2) * 0.5f;
    x23 = (x2 + x3) * 0.5f;
    y23 = (y2 + y3) * 0.5f;
    x34 = (x3 + x4) * 0.5f;
    y34 = (y3 + y4) * 0.5f;
    x123 = (x12 + x23) * 0.5f;
    y123 = (y12 + y23) * 0.5f;
    dx = x4 - x1;
    dy = y4 - y1;
    d2 = absf(((x2 - x4) * dy - (y2 - y4) * dx));
    d3 = absf(((x3 - x4) * dy - (y3 - y4) * dx));

    if ((d2 + d3) * (d2 + d3) < NVG_TESSTOL * (dx * dx + dy * dy))
    {

        addpoint(ctx, x4, y4);

        return;

    }

    x234 = (x23 + x34) * 0.5f;
    y234 = (y23 + y34) * 0.5f;
    x1234 = (x123 + x234) * 0.5f;
    y1234 = (y123 + y234) * 0.5f;

    tesselatebezier(ctx, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
    tesselatebezier(ctx, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);

}

void nvg_flatten(struct nvg_context *ctx)
{

    unsigned int i;

    if (ctx->npaths > 0)
        return;

    for (i = 0; i < ctx->ncommands;)
    {

        int cmd = (int)ctx->commands[i];
        struct nvg_point *last;
        float *p;

        switch (cmd)
        {

        case NVG_MOVETO:
            addpath(ctx);

            p = &ctx->commands[i + 1];

            addpoint(ctx, p[0], p[1]);

            i += 3;

            break;

        case NVG_LINETO:
            p = &ctx->commands[i + 1];

            addpoint(ctx, p[0], p[1]);

            i += 3;

            break;

        case NVG_BEZIERTO:
            last = lastpoint(ctx);

            if (last)
            {

                float *cp1 = &ctx->commands[i + 1];
                float *cp2 = &ctx->commands[i + 3];

                p = &ctx->commands[i + 5];

                tesselatebezier(ctx, last->x, last->y, cp1[0], cp1[1], cp2[0], cp2[1], p[0], p[1], 0);

            }

            i += 7;

            break;

        case NVG_CLOSE:
            closepath(ctx);

            i++;

            break;

        case NVG_WINDING:
            pathwinding(ctx, (int)ctx->commands[i + 1]);

            i += 2;

            break;

        default:
            i++;

        }

    }

    ctx->bounds[0] = ctx->bounds[1] = 1e6f;
    ctx->bounds[2] = ctx->bounds[3] = -1e6f;

    for (i = 0; i < ctx->npaths; i++)
    {

        struct nvg_path *path = &ctx->paths[i];
        struct nvg_point *pts = &ctx->points[path->first];
        struct nvg_point *p0 = &pts[path->count - 1];
        struct nvg_point *p1 = &pts[0];
        unsigned int j;

        if (pointequals(p0->x, p0->y, p1->x, p1->y, NVG_DISTTOL))
        {

            path->count--;
            p0 = &pts[path->count - 1];
            path->closed = 1;

        }

        if (path->count > 2)
        {

            float area = polyarea(pts, path->count);

            if (path->winding == NVG_CCW && area < 0.0f)
                polyreverse(pts, path->count);

            if (path->winding == NVG_CW && area > 0.0f)
                polyreverse(pts, path->count);

        }

        for (j = 0; j < path->count; j++)
        {

            p0->dx = p1->x - p0->x;
            p0->dy = p1->y - p0->y;
            p0->len = normalize(&p0->dx, &p0->dy);
            ctx->bounds[0] = minf(ctx->bounds[0], p0->x);
            ctx->bounds[1] = minf(ctx->bounds[1], p0->y);
            ctx->bounds[2] = maxf(ctx->bounds[2], p0->x);
            ctx->bounds[3] = maxf(ctx->bounds[3], p0->y);
            p0 = p1++;

        }

    }

}

void nvg_expand(struct nvg_context *ctx)
{

    struct nvg_vertex *verts = ctx->verts;
    unsigned int i;

    for (i = 0; i < ctx->npaths; i++)
    {

        struct nvg_path *path = &ctx->paths[i];
        struct nvg_point *pts = &ctx->points[path->first];
        struct nvg_vertex *dst = verts;
        unsigned int j;

        path->fill = dst;

        for (j = 0; j < path->count; j++)
            nvg_setvertex(dst++, pts[j].x, pts[j].y, 0.5f, 1);

        path->nfill = (int)(dst - verts);
        verts = dst;

    }

}

void nvg_path_moveto(struct nvg_context *ctx, float x, float y)
{

    float vals[] = { NVG_MOVETO, x, y };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_lineto(struct nvg_context *ctx, float x, float y)
{

    float vals[] = { NVG_LINETO, x, y };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_bezierto(struct nvg_context *ctx, float c1x, float c1y, float c2x, float c2y, float x, float y)
{

    float vals[] = { NVG_BEZIERTO, c1x, c1y, c2x, c2y, x, y };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_begin(struct nvg_context *ctx)
{

    ctx->ncommands = 0;
    ctx->npoints = 0;
    ctx->npaths = 0;

}

void nvg_path_close(struct nvg_context *ctx)
{

    float vals[] = { NVG_CLOSE };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_solid(struct nvg_context *ctx)
{

    float vals[] = { NVG_WINDING, (float)NVG_CCW };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_hole(struct nvg_context *ctx)
{

    float vals[] = { NVG_WINDING, (float)NVG_CW };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_rect(struct nvg_context *ctx, float x, float y, float w, float h)
{

    float vals[] = {
        NVG_MOVETO, x, y,
        NVG_LINETO, x, y + h,
        NVG_LINETO, x + w, y + h,
        NVG_LINETO, x + w, y,
        NVG_CLOSE
    };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_roundedrect(struct nvg_context *ctx, float x, float y, float w, float h, float r)
{

    nvg_path_roundedrectvarying(ctx, x, y, w, h, r, r, r, r);

}

void nvg_path_roundedrectvarying(struct nvg_context *ctx, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft)
{

    float halfw = absf(w) * 0.5f;
    float halfh = absf(h) * 0.5f;
    float rxBL = minf(radBottomLeft, halfw) * signf(w), ryBL = minf(radBottomLeft, halfh) * signf(h);
    float rxBR = minf(radBottomRight, halfw) * signf(w), ryBR = minf(radBottomRight, halfh) * signf(h);
    float rxTR = minf(radTopRight, halfw) * signf(w), ryTR = minf(radTopRight, halfh) * signf(h);
    float rxTL = minf(radTopLeft, halfw) * signf(w), ryTL = minf(radTopLeft, halfh) * signf(h);
    float vals[] = {
        NVG_MOVETO, x, y + ryTL,
        NVG_LINETO, x, y + h - ryBL,
        NVG_BEZIERTO, x, y + h - ryBL * (1 - NVG_KAPPA90), x + rxBL * (1 - NVG_KAPPA90), y + h, x + rxBL, y + h,
        NVG_LINETO, x + w - rxBR, y + h,
        NVG_BEZIERTO, x + w - rxBR * (1 - NVG_KAPPA90), y + h, x + w, y + h - ryBR * (1 - NVG_KAPPA90), x + w, y + h - ryBR,
        NVG_LINETO, x + w, y + ryTR,
        NVG_BEZIERTO, x + w, y + ryTR * (1 - NVG_KAPPA90), x + w - rxTR * (1 - NVG_KAPPA90), y, x + w - rxTR, y,
        NVG_LINETO, x + rxTL, y,
        NVG_BEZIERTO, x + rxTL * (1 - NVG_KAPPA90), y, x, y + ryTL * (1 - NVG_KAPPA90), x, y + ryTL,
        NVG_CLOSE
    };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_ellipse(struct nvg_context *ctx, float cx, float cy, float rx, float ry)
{

    float vals[] = {
        NVG_MOVETO, cx - rx, cy,
        NVG_BEZIERTO, cx - rx, cy + ry * NVG_KAPPA90, cx - rx * NVG_KAPPA90, cy + ry, cx, cy + ry,
        NVG_BEZIERTO, cx + rx * NVG_KAPPA90, cy + ry, cx + rx, cy + ry * NVG_KAPPA90, cx + rx, cy,
        NVG_BEZIERTO, cx + rx, cy - ry * NVG_KAPPA90, cx + rx * NVG_KAPPA90, cy - ry, cx, cy - ry,
        NVG_BEZIERTO, cx - rx * NVG_KAPPA90, cy - ry, cx - rx, cy - ry * NVG_KAPPA90, cx - rx, cy,
        NVG_CLOSE
    };

    appendcommand(ctx, vals, NVG_COUNTOF(vals));

}

void nvg_path_circle(struct nvg_context *ctx, float cx, float cy, float r)
{

    nvg_path_ellipse(ctx, cx, cy, r, r);

}

void nvg_reset(struct nvg_context *ctx)
{

    nvg_xform_identity(ctx->xform);

}

