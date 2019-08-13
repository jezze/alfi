#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "nvg.h"

#define NVG_PI 3.14159265358979323846264338327f
#define NVG_KAPPA90 0.5522847493f
#define NVG_COUNTOF(arr) (sizeof (arr) / sizeof (0[arr]))

static int mini(int a, int b)
{

    return a < b ? a : b;

}

static int maxi(int a, int b)
{

    return a > b ? a : b;

}

static int clampi(int a, int mn, int mx)
{

    return a < mn ? mn : (a > mx ? mx : a);

}

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

static float clampf(float a, float mn, float mx)
{

    return a < mn ? mn : (a > mx ? mx : a);

}

static float cross(float dx0, float dy0, float dx1, float dy1)
{

    return dx1 * dy0 - dx0 * dy1;

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

struct nvg_color nvgRGB(unsigned char r, unsigned char g, unsigned char b)
{

    return nvgRGBA(r, g, b, 255);

}

struct nvg_color nvgRGBf(float r, float g, float b)
{

    return nvgRGBAf(r, g, b, 1.0f);

}

struct nvg_color nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{

    struct nvg_color color;

    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = a / 255.0f;

    return color;

}

struct nvg_color nvgRGBAf(float r, float g, float b, float a)
{

    struct nvg_color color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;

}

struct nvg_color nvgTransRGBA(struct nvg_color c, unsigned char a)
{

    c.a = a / 255.0f;

    return c;

}

struct nvg_color nvgTransRGBAf(struct nvg_color c, float a)
{

    c.a = a;

    return c;

}

struct nvg_color nvgLerpRGBA(struct nvg_color c0, struct nvg_color c1, float u)
{

    struct nvg_color c;

    u = clampf(u, 0.0f, 1.0f);
    c.r = c0.r * (1.0f - u) + c1.r * u;
    c.g = c0.g * (1.0f - u) + c1.g * u;
    c.b = c0.b * (1.0f - u) + c1.b * u;
    c.a = c0.a * (1.0f - u) + c1.a * u;

    return c;

}

struct nvg_color nvgHSL(float h, float s, float l)
{

    return nvgHSLA(h, s, l, 255);

}

static float hue(float h, float m1, float m2)
{

    if (h < 0)
        h += 1;

    if (h > 1)
        h -= 1;

    if (h < 1.0f / 6.0f)
        return m1 + (m2 - m1) * h * 6.0f;
    else if (h < 3.0f / 6.0f)
        return m2;
    else if (h < 4.0f / 6.0f)
        return m1 + (m2 - m1) * (2.0f / 3.0f - h) * 6.0f;

    return m1;

}

struct nvg_color nvgHSLA(float h, float s, float l, unsigned char a)
{

    float m1, m2;
    struct nvg_color col;

    h = fmodf(h, 1.0f);

    if (h < 0.0f)
        h += 1.0f;

    s = clampf(s, 0.0f, 1.0f);
    l = clampf(l, 0.0f, 1.0f);
    m2 = l <= 0.5f ? (l * (1 + s)) : (l + s - l * s);
    m1 = 2 * l - m2;
    col.r = clampf(hue(h + 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
    col.g = clampf(hue(h, m1, m2), 0.0f, 1.0f);
    col.b = clampf(hue(h - 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
    col.a = a / 255.0f;

    return col;

}

void nvgTransformIdentity(float *t)
{

    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvgTransformTranslate(float *t, float tx, float ty)
{

    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = tx;
    t[5] = ty;

}

void nvgTransformScale(float *t, float sx, float sy)
{

    t[0] = sx;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = sy;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvgTransformRotate(float *t, float a)
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

void nvgTransformSkewX(float *t, float a)
{

    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = tanf(a);
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvgTransformSkewY(float *t, float a)
{

    t[0] = 1.0f;
    t[1] = tanf(a);
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;

}

void nvgTransformMultiply(float *t, const float *s)
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

void nvgTransformPremultiply(float *t, const float *s)
{

    float s2[6];

    memcpy(s2, s, sizeof (float) * 6);
    nvgTransformMultiply(s2, t);
    memcpy(t, s2, sizeof (float) * 6);

}

int nvgTransformInverse(float *inv, const float *t)
{

    double det = (double)t[0] * t[3] - (double)t[2] * t[1];
    double invdet;

    if (det > -1e-6 && det < 1e-6)
    {

        nvgTransformIdentity(inv);

        return 0;

    }

    invdet = 1.0 / det;
    inv[0] = (float)(t[3] * invdet);
    inv[2] = (float)(-t[2] * invdet);
    inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
    inv[1] = (float)(-t[1] * invdet);
    inv[3] = (float)(t[0] * invdet);
    inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);

    return 1;

}

void nvgTransformPoint(float *dx, float *dy, const float *t, float sx, float sy)
{

    *dx = sx * t[0] + sy * t[2] + t[4];
    *dy = sx * t[1] + sy * t[3] + t[5];

}

float nvgDegToRad(float deg)
{

    return deg / 180.0f * NVG_PI;

}

float nvgRadToDeg(float rad)
{

    return rad / NVG_PI * 180.0f;

}

static void setPaintColor(struct nvg_paint *p, struct nvg_color color)
{

    memset(p, 0, sizeof (struct nvg_paint));
    nvgTransformIdentity(p->xform);

    p->radius = 0.0f;
    p->feather = 1.0f;
    p->innerColor = color;
    p->outerColor = color;

}

void nvgTransform(struct nvg_context *ctx, float a, float b, float c, float d, float e, float f)
{

    float t[6] = { a, b, c, d, e, f };

    nvgTransformPremultiply(ctx->state.xform, t);

}

void nvgTranslate(struct nvg_context *ctx, float x, float y)
{

    float t[6];

    nvgTransformTranslate(t, x,y);
    nvgTransformPremultiply(ctx->state.xform, t);

}

void nvgRotate(struct nvg_context *ctx, float angle)
{

    float t[6];

    nvgTransformRotate(t, angle);
    nvgTransformPremultiply(ctx->state.xform, t);

}

void nvgSkewX(struct nvg_context *ctx, float angle)
{

    float t[6];

    nvgTransformSkewX(t, angle);
    nvgTransformPremultiply(ctx->state.xform, t);

}

void nvgSkewY(struct nvg_context *ctx, float angle)
{

    float t[6];

    nvgTransformSkewY(t, angle);
    nvgTransformPremultiply(ctx->state.xform, t);

}

void nvgScale(struct nvg_context *ctx, float x, float y)
{

    float t[6];

    nvgTransformScale(t, x, y);
    nvgTransformPremultiply(ctx->state.xform, t);

}

void nvg_setstrokecolor(struct nvg_context *ctx, struct nvg_color color)
{

    setPaintColor(&ctx->state.stroke, color);

}

void nvg_setstrokepaint(struct nvg_context *ctx, struct nvg_paint paint)
{

    ctx->state.stroke = paint;

    nvgTransformMultiply(ctx->state.stroke.xform, ctx->state.xform);

}

void nvg_setfillcolor(struct nvg_context *ctx, struct nvg_color color)
{

    setPaintColor(&ctx->state.fill, color);

}

void nvg_setfillpaint(struct nvg_context *ctx, struct nvg_paint paint)
{

    ctx->state.fill = paint;

    nvgTransformMultiply(ctx->state.fill.xform, ctx->state.xform);

}

void nvgLinearGradient(struct nvg_paint *p, float sx, float sy, float ex, float ey, struct nvg_color icol, struct nvg_color ocol)
{

    float dx, dy, d;
    const float large = 1e5;

    memset(p, 0, sizeof (struct nvg_paint));

    dx = ex - sx;
    dy = ey - sy;
    d = sqrtf(dx * dx + dy * dy);

    if (d > 0.0001f)
    {

        dx /= d;
        dy /= d;

    }

    else
    {

        dx = 0;
        dy = 1;

    }

    p->xform[0] = dy;
    p->xform[1] = -dx;
    p->xform[2] = dx;
    p->xform[3] = dy;
    p->xform[4] = sx - dx * large;
    p->xform[5] = sy - dy * large;
    p->extent[0] = large;
    p->extent[1] = large + d * 0.5f;
    p->radius = 0.0f;
    p->feather = maxf(1.0f, d);
    p->innerColor = icol;
    p->outerColor = ocol;

}

void nvgRadialGradient(struct nvg_paint *p, float cx, float cy, float inr, float outr, struct nvg_color icol, struct nvg_color ocol)
{

    float r = (inr + outr) * 0.5f;
    float f = (outr - inr);

    memset(p, 0, sizeof (struct nvg_paint));
    nvgTransformIdentity(p->xform);

    p->xform[4] = cx;
    p->xform[5] = cy;
    p->extent[0] = r;
    p->extent[1] = r;
    p->radius = r;
    p->feather = maxf(1.0f, f);
    p->innerColor = icol;
    p->outerColor = ocol;

}

void nvgBoxGradient(struct nvg_paint *p, float x, float y, float w, float h, float r, float f, struct nvg_color icol, struct nvg_color ocol)
{

    memset(p, 0, sizeof (struct nvg_paint));
    nvgTransformIdentity(p->xform);

    p->xform[4] = x + w * 0.5f;
    p->xform[5] = y + h * 0.5f;
    p->extent[0] = w * 0.5f;
    p->extent[1] = h * 0.5f;
    p->radius = r;
    p->feather = maxf(1.0f, f);
    p->innerColor = icol;
    p->outerColor = ocol;

}

void nvgImagePattern(struct nvg_paint *p, float cx, float cy, float w, float h, float angle, int image, float alpha)
{

    memset(p, 0, sizeof (struct nvg_paint));
    nvgTransformRotate(p->xform, angle);

    p->xform[4] = cx;
    p->xform[5] = cy;
    p->extent[0] = w;
    p->extent[1] = h;
    p->image = image;
    p->innerColor = p->outerColor = nvgRGBAf(1, 1, 1, alpha);

}

void nvgScissor(struct nvg_context *ctx, float x, float y, float w, float h)
{

    w = maxf(0.0f, w);
    h = maxf(0.0f, h);

    nvgTransformIdentity(ctx->state.scissor.xform);

    ctx->state.scissor.xform[4] = x + w * 0.5f;
    ctx->state.scissor.xform[5] = y + h * 0.5f;

    nvgTransformMultiply(ctx->state.scissor.xform, ctx->state.xform);

    ctx->state.scissor.extent[0] = w * 0.5f;
    ctx->state.scissor.extent[1] = h * 0.5f;

}

static void isectRects(float *dst, float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{

    float minx = maxf(ax, bx);
    float miny = maxf(ay, by);
    float maxx = minf(ax + aw, bx + bw);
    float maxy = minf(ay + ah, by + bh);

    dst[0] = minx;
    dst[1] = miny;
    dst[2] = maxf(0.0f, maxx - minx);
    dst[3] = maxf(0.0f, maxy - miny);

}

void nvgIntersectScissor(struct nvg_context *ctx, float x, float y, float w, float h)
{

    float pxform[6], invxorm[6];
    float rect[4];
    float ex, ey, tex, tey;

    if (ctx->state.scissor.extent[0] < 0)
    {

        nvgScissor(ctx, x, y, w, h);

        return;

    }

    memcpy(pxform, ctx->state.scissor.xform, sizeof (float) * 6);

    ex = ctx->state.scissor.extent[0];
    ey = ctx->state.scissor.extent[1];

    nvgTransformInverse(invxorm, ctx->state.xform);
    nvgTransformMultiply(pxform, invxorm);

    tex = ex * absf(pxform[0]) + ey * absf(pxform[2]);
    tey = ex * absf(pxform[1]) + ey * absf(pxform[3]);

    isectRects(rect, pxform[4] - tex, pxform[5] - tey, tex * 2, tey * 2, x, y, w, h);
    nvgScissor(ctx, rect[0], rect[1], rect[2], rect[3]);

}

void nvgResetScissor(struct nvg_context *ctx)
{

    memset(ctx->state.scissor.xform, 0, sizeof (ctx->state.scissor.xform));

    ctx->state.scissor.extent[0] = -1.0f;
    ctx->state.scissor.extent[1] = -1.0f;

}

void nvgGlobalCompositeBlendFunc(struct nvg_context *ctx, int sfactor, int dfactor)
{

    nvgGlobalCompositeBlendFuncSeparate(ctx, sfactor, dfactor, sfactor, dfactor);

}

void nvgGlobalCompositeBlendFuncSeparate(struct nvg_context *ctx, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha)
{

    ctx->state.compositeoperation.srcRGB = srcRGB;
    ctx->state.compositeoperation.dstRGB = dstRGB;
    ctx->state.compositeoperation.srcAlpha = srcAlpha;
    ctx->state.compositeoperation.dstAlpha = dstAlpha;

}

static int ptEquals(float x1, float y1, float x2, float y2, float tol)
{

    float dx = x2 - x1;
    float dy = y2 - y1;

    return dx * dx + dy * dy < tol * tol;

}

static float distPtSeg(float x, float y, float px, float py, float qx, float qy)
{

    float pqx, pqy, dx, dy, d, t;

    pqx = qx - px;
    pqy = qy - py;
    dx = x - px;
    dy = y - py;
    d = pqx * pqx + pqy * pqy;
    t = pqx * dx + pqy * dy;

    if (d > 0)
        t /= d;

    if (t < 0)
        t = 0;
    else if (t > 1)
        t = 1;

    dx = px + t * pqx - x;
    dy = py + t * pqy - y;

    return dx * dx + dy * dy;

}

static void appendCommands(struct nvg_context *ctx, float *vals, int nvals)
{

    int i;

    if ((int)vals[0] != NVG_CLOSE && (int)vals[0] != NVG_WINDING)
    {

        ctx->commandx = vals[nvals - 2];
        ctx->commandy = vals[nvals - 1];

    }

    i = 0;

    while (i < nvals)
    {

        int cmd = (int)vals[i];

        switch (cmd)
        {

        case NVG_MOVETO:
            nvgTransformPoint(&vals[i + 1], &vals[i + 2], ctx->state.xform, vals[i + 1], vals[i + 2]);

            i += 3;

            break;

        case NVG_LINETO:
            nvgTransformPoint(&vals[i + 1], &vals[i + 2], ctx->state.xform, vals[i + 1],vals[i + 2]);

            i += 3;

            break;

        case NVG_BEZIERTO:
            nvgTransformPoint(&vals[i + 1],&vals[i + 2], ctx->state.xform, vals[i + 1], vals[i + 2]);
            nvgTransformPoint(&vals[i + 3],&vals[i + 4], ctx->state.xform, vals[i + 3], vals[i + 4]);
            nvgTransformPoint(&vals[i + 5],&vals[i + 6], ctx->state.xform, vals[i + 5], vals[i + 6]);

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


static void clearPathCache(struct nvg_context *ctx)
{

    ctx->cache.npoints = 0;
    ctx->cache.npaths = 0;

}

static struct nvg_path *lastPath(struct nvg_context *ctx)
{

    if (ctx->cache.npaths > 0)
        return &ctx->cache.paths[ctx->cache.npaths - 1];

    return NULL;

}

static void addPath(struct nvg_context *ctx)
{

    struct nvg_path *path = &ctx->cache.paths[ctx->cache.npaths];

    memset(path, 0, sizeof (struct nvg_path));

    path->first = ctx->cache.npoints;
    path->winding = NVG_CCW;
    ctx->cache.npaths++;

}

static struct nvg_point *lastPoint(struct nvg_context *ctx)
{

    if (ctx->cache.npoints > 0)
        return &ctx->cache.points[ctx->cache.npoints - 1];

    return NULL;

}

static void addPoint(struct nvg_context *ctx, float x, float y, int flags)
{

    struct nvg_path *path = lastPath(ctx);
    struct nvg_point *pt;

    if (!path)
        return;

    if (path->count > 0 && ctx->cache.npoints > 0)
    {

        pt = lastPoint(ctx);

        if (ptEquals(pt->x,pt->y, x,y, ctx->distTol))
        {

            pt->flags |= flags;

            return;

        }

    }

    pt = &ctx->cache.points[ctx->cache.npoints];

    memset(pt, 0, sizeof (struct nvg_point));

    pt->x = x;
    pt->y = y;
    pt->flags = (unsigned char)flags;
    ctx->cache.npoints++;
    path->count++;

}

static void closePath(struct nvg_context *ctx)
{

    struct nvg_path *path = lastPath(ctx);

    if (!path)
        return;

    path->closed = 1;

}

static void pathWinding(struct nvg_context *ctx, int winding)
{

    struct nvg_path *path = lastPath(ctx);

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

static float polyArea(struct nvg_point *pts, int npts)
{

    float area = 0;
    int i;

    for (i = 2; i < npts; i++)
    {

        struct nvg_point *a = &pts[0];
        struct nvg_point *b = &pts[i - 1];
        struct nvg_point *c = &pts[i];

        area += triarea2(a->x, a->y, b->x, b->y, c->x, c->y);

    }

    return area * 0.5f;

}

static void polyReverse(struct nvg_point *pts, int npts)
{

    struct nvg_point tmp;
    int i = 0, j = npts - 1;

    while (i < j)
    {

        tmp = pts[i];
        pts[i] = pts[j];
        pts[j] = tmp;
        i++;
        j--;

    }

}

static void vset(struct nvg_vertex *vtx, float x, float y, float u, float v)
{

    vtx->x = x;
    vtx->y = y;
    vtx->u = u;
    vtx->v = v;

}

static void tesselateBezier(struct nvg_context *ctx, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int level, int type)
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

    if ((d2 + d3) * (d2 + d3) < ctx->tessTol * (dx * dx + dy * dy))
    {

        addPoint(ctx, x4, y4, type);

        return;

    }

    x234 = (x23 + x34) * 0.5f;
    y234 = (y23 + y34) * 0.5f;
    x1234 = (x123 + x234) * 0.5f;
    y1234 = (y123 + y234) * 0.5f;

    tesselateBezier(ctx, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, 0);
    tesselateBezier(ctx, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, type);

}

void nvg_flatten_paths(struct nvg_context *ctx)
{

    struct nvg_pathcache *cache = &ctx->cache;
    struct nvg_point *last;
    struct nvg_point *p0;
    struct nvg_point *p1;
    struct nvg_point *pts;
    struct nvg_path *path;
    float *cp1;
    float *cp2;
    float *p;
    float area;
    int i, j;

    if (cache->npaths > 0)
        return;

    i = 0;

    while (i < ctx->ncommands)
    {

        int cmd = (int)ctx->commands[i];

        switch (cmd)
        {

        case NVG_MOVETO:
            addPath(ctx);

            p = &ctx->commands[i + 1];

            addPoint(ctx, p[0], p[1], NVG_PT_CORNER);

            i += 3;

            break;

        case NVG_LINETO:
            p = &ctx->commands[i + 1];

            addPoint(ctx, p[0], p[1], NVG_PT_CORNER);

            i += 3;

            break;

        case NVG_BEZIERTO:
            last = lastPoint(ctx);

            if (last)
            {

                cp1 = &ctx->commands[i + 1];
                cp2 = &ctx->commands[i + 3];
                p = &ctx->commands[i + 5];

                tesselateBezier(ctx, last->x, last->y, cp1[0], cp1[1], cp2[0], cp2[1], p[0], p[1], 0, NVG_PT_CORNER);

            }

            i += 7;

            break;

        case NVG_CLOSE:
            closePath(ctx);

            i++;

            break;

        case NVG_WINDING:
            pathWinding(ctx, (int)ctx->commands[i + 1]);

            i += 2;

            break;

        default:
            i++;

        }

    }

    cache->bounds[0] = cache->bounds[1] = 1e6f;
    cache->bounds[2] = cache->bounds[3] = -1e6f;

    for (j = 0; j < cache->npaths; j++)
    {

        path = &cache->paths[j];
        pts = &cache->points[path->first];
        p0 = &pts[path->count - 1];
        p1 = &pts[0];

        if (ptEquals(p0->x, p0->y, p1->x, p1->y, ctx->distTol))
        {

            path->count--;
            p0 = &pts[path->count - 1];
            path->closed = 1;

        }

        if (path->count > 2)
        {

            area = polyArea(pts, path->count);

            if (path->winding == NVG_CCW && area < 0.0f)
                polyReverse(pts, path->count);

            if (path->winding == NVG_CW && area > 0.0f)
                polyReverse(pts, path->count);

        }

        for (i = 0; i < path->count; i++)
        {

            p0->dx = p1->x - p0->x;
            p0->dy = p1->y - p0->y;
            p0->len = normalize(&p0->dx, &p0->dy);
            cache->bounds[0] = minf(cache->bounds[0], p0->x);
            cache->bounds[1] = minf(cache->bounds[1], p0->y);
            cache->bounds[2] = maxf(cache->bounds[2], p0->x);
            cache->bounds[3] = maxf(cache->bounds[3], p0->y);
            p0 = p1++;

        }

    }

}

static int curveDivs(float r, float arc, float tol)
{

    float da = acosf(r / (r + tol)) * 2.0f;

    return maxi(2, (int)ceilf(arc / da));

}

static void chooseBevel(int bevel, struct nvg_point *p0, struct nvg_point *p1, float w, float *x0, float *y0, float *x1, float *y1)
{

    if (bevel)
    {

        *x0 = p1->x + p0->dy * w;
        *y0 = p1->y - p0->dx * w;
        *x1 = p1->x + p1->dy * w;
        *y1 = p1->y - p1->dx * w;

    }

    else
    {

        *x0 = p1->x + p1->dmx * w;
        *y0 = p1->y + p1->dmy * w;
        *x1 = p1->x + p1->dmx * w;
        *y1 = p1->y + p1->dmy * w;

    }

}

static struct nvg_vertex *roundJoin(struct nvg_vertex *dst, struct nvg_point *p0, struct nvg_point *p1, float lw, float rw, float lu, float ru, int ncap, float fringe)
{

    int i, n;
    float dlx0 = p0->dy;
    float dly0 = -p0->dx;
    float dlx1 = p1->dy;
    float dly1 = -p1->dx;

    if (p1->flags & NVG_PT_LEFT)
    {

        float lx0, ly0, lx1, ly1, a0, a1;

        chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);

        a0 = atan2f(-dly0, -dlx0);
        a1 = atan2f(-dly1, -dlx1);

        if (a1 > a0)
            a1 -= NVG_PI * 2;

        vset(dst, lx0, ly0, lu, 1);

        dst++;

        vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);

        dst++;

        n = clampi((int)ceilf(((a0 - a1) / NVG_PI) * ncap), 2, ncap);

        for (i = 0; i < n; i++)
        {

            float u = i / (float)(n - 1);
            float a = a0 + u * (a1-a0);
            float rx = p1->x + cosf(a) * rw;
            float ry = p1->y + sinf(a) * rw;

            vset(dst, p1->x, p1->y, 0.5f, 1);

            dst++;

            vset(dst, rx, ry, ru, 1);

            dst++;

        }

        vset(dst, lx1, ly1, lu, 1);

        dst++;

        vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);

        dst++;

    }

    else
    {

        float rx0, ry0, rx1, ry1, a0, a1;

        chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);

        a0 = atan2f(dly0, dlx0);
        a1 = atan2f(dly1, dlx1);

        if (a1 < a0)
            a1 += NVG_PI * 2;

        vset(dst, p1->x + dlx0 * rw, p1->y + dly0 * rw, lu, 1);

        dst++;

        vset(dst, rx0, ry0, ru, 1);

        dst++;
        n = clampi((int)ceilf(((a1 - a0) / NVG_PI) * ncap), 2, ncap);

        for (i = 0; i < n; i++)
        {

            float u = i / (float)(n - 1);
            float a = a0 + u*(a1 - a0);
            float lx = p1->x + cosf(a) * lw;
            float ly = p1->y + sinf(a) * lw;

            vset(dst, lx, ly, lu, 1);

            dst++;

            vset(dst, p1->x, p1->y, 0.5f, 1);

            dst++;

        }

        vset(dst, p1->x + dlx1 * rw, p1->y + dly1 * rw, lu, 1);

        dst++;

        vset(dst, rx1, ry1, ru, 1);

        dst++;

    }

    return dst;

}

static struct nvg_vertex *bevelJoin(struct nvg_vertex *dst, struct nvg_point *p0, struct nvg_point *p1, float lw, float rw, float lu, float ru, float fringe)
{

    float rx0, ry0, rx1, ry1;
    float lx0, ly0, lx1, ly1;
    float dlx0 = p0->dy;
    float dly0 = -p0->dx;
    float dlx1 = p1->dy;
    float dly1 = -p1->dx;

    if (p1->flags & NVG_PT_LEFT)
    {

        chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, lw, &lx0, &ly0, &lx1, &ly1);
        vset(dst, lx0, ly0, lu, 1);

        dst++;

        vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);

        dst++;

        if (p1->flags & NVG_PT_BEVEL)
        {

            vset(dst, lx0, ly0, lu, 1);

            dst++;

            vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);

            dst++;

            vset(dst, lx1, ly1, lu, 1);

            dst++;

            vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
            
            dst++;

        }

        else
        {

            rx0 = p1->x - p1->dmx * rw;
            ry0 = p1->y - p1->dmy * rw;

            vset(dst, p1->x, p1->y, 0.5f, 1);

            dst++;

            vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);

            dst++;

            vset(dst, rx0, ry0, ru, 1);

            dst++;

            vset(dst, rx0, ry0, ru, 1);

            dst++;

            vset(dst, p1->x, p1->y, 0.5f, 1);

            dst++;

            vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);

            dst++;

        }

        vset(dst, lx1, ly1, lu, 1);

        dst++;

        vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);

        dst++;

    }

    else
    {

        chooseBevel(p1->flags & NVG_PR_INNERBEVEL, p0, p1, -rw, &rx0, &ry0, &rx1, &ry1);
        vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);

        dst++;

        vset(dst, rx0, ry0, ru, 1);

        dst++;

        if (p1->flags & NVG_PT_BEVEL)
        {

            vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);

            dst++;

            vset(dst, rx0, ry0, ru, 1);

            dst++;

            vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);

            dst++;

            vset(dst, rx1, ry1, ru, 1);

            dst++;

        }

        else
        {

            lx0 = p1->x + p1->dmx * lw;
            ly0 = p1->y + p1->dmy * lw;

            vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);

            dst++;

            vset(dst, p1->x, p1->y, 0.5f, 1);

            dst++;

            vset(dst, lx0, ly0, lu, 1);

            dst++;

            vset(dst, lx0, ly0, lu, 1);

            dst++;

            vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);

            dst++;

            vset(dst, p1->x, p1->y, 0.5f, 1);

            dst++;

        }

        vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);

        dst++;

        vset(dst, rx1, ry1, ru, 1);

        dst++;

    }

    return dst;

}

static struct nvg_vertex *buttCapStart(struct nvg_vertex *dst, struct nvg_point *p, float dx, float dy, float w, float d, float aa, float u0, float u1)
{

    float px = p->x - dx * d;
    float py = p->y - dy * d;
    float dlx = dy;
    float dly = -dx;

    vset(dst, px + dlx * w - dx * aa, py + dly * w - dy * aa, u0, 0);

    dst++;

    vset(dst, px - dlx * w - dx * aa, py - dly * w - dy * aa, u1, 0);

    dst++;

    vset(dst, px + dlx * w, py + dly * w, u0, 1);

    dst++;

    vset(dst, px - dlx * w, py - dly * w, u1, 1);

    dst++;

    return dst;

}

static struct nvg_vertex *buttCapEnd(struct nvg_vertex *dst, struct nvg_point *p, float dx, float dy, float w, float d, float aa, float u0, float u1)
{

    float px = p->x + dx * d;
    float py = p->y + dy * d;
    float dlx = dy;
    float dly = -dx;

    vset(dst, px + dlx * w, py + dly * w, u0, 1);

    dst++;

    vset(dst, px - dlx * w, py - dly * w, u1, 1);

    dst++;

    vset(dst, px + dlx * w + dx * aa, py + dly * w + dy * aa, u0, 0);

    dst++;

    vset(dst, px - dlx * w + dx * aa, py - dly * w + dy * aa, u1, 0);

    dst++;

    return dst;

}

static struct nvg_vertex *roundCapStart(struct nvg_vertex *dst, struct nvg_point *p, float dx, float dy, float w, int ncap, float aa, float u0, float u1)
{

    int i;
    float px = p->x;
    float py = p->y;
    float dlx = dy;
    float dly = -dx;

    for (i = 0; i < ncap; i++)
    {

        float a = i / (float)(ncap - 1) * NVG_PI;
        float ax = cosf(a) * w, ay = sinf(a) * w;

        vset(dst, px - dlx * ax - dx * ay, py - dly * ax - dy * ay, u0, 1);

        dst++;

        vset(dst, px, py, 0.5f, 1);

        dst++;

    }

    vset(dst, px + dlx * w, py + dly * w, u0, 1);

    dst++;

    vset(dst, px - dlx * w, py - dly * w, u1, 1);

    dst++;

    return dst;
    
}

static struct nvg_vertex *roundCapEnd(struct nvg_vertex *dst, struct nvg_point *p, float dx, float dy, float w, int ncap, float aa, float u0, float u1)
{

    int i;
    float px = p->x;
    float py = p->y;
    float dlx = dy;
    float dly = -dx;

    vset(dst, px + dlx * w, py + dly * w, u0, 1);

    dst++;

    vset(dst, px - dlx * w, py - dly * w, u1, 1);

    dst++;

    for (i = 0; i < ncap; i++)
    {

        float a = i / (float)(ncap - 1) * NVG_PI;
        float ax = cosf(a) * w, ay = sinf(a) * w;

        vset(dst, px, py, 0.5f, 1);

        dst++;

        vset(dst, px - dlx * ax + dx*ay, py - dly * ax + dy * ay, u0, 1);
        
        dst++;

    }

    return dst;

}

static void calculateJoins(struct nvg_context *ctx, float w, int linejoin, float miterLimit)
{

    struct nvg_pathcache *cache = &ctx->cache;
    int i, j;
    float iw = 0.0f;

    if (w > 0.0f)
        iw = 1.0f / w;

    for (i = 0; i < cache->npaths; i++)
    {

        struct nvg_path *path = &cache->paths[i];
        struct nvg_point *pts = &cache->points[path->first];
        struct nvg_point *p0 = &pts[path->count - 1];
        struct nvg_point *p1 = &pts[0];
        int nleft = 0;

        path->nbevel = 0;

        for (j = 0; j < path->count; j++)
        {

            float dlx0, dly0, dlx1, dly1, dmr2, cross, limit;

            dlx0 = p0->dy;
            dly0 = -p0->dx;
            dlx1 = p1->dy;
            dly1 = -p1->dx;
            p1->dmx = (dlx0 + dlx1) * 0.5f;
            p1->dmy = (dly0 + dly1) * 0.5f;
            dmr2 = p1->dmx * p1->dmx + p1->dmy * p1->dmy;

            if (dmr2 > 0.000001f)
            {

                float scale = 1.0f / dmr2;

                if (scale > 600.0f)
                    scale = 600.0f;

                p1->dmx *= scale;
                p1->dmy *= scale;

            }

            p1->flags = (p1->flags & NVG_PT_CORNER) ? NVG_PT_CORNER : 0;
            cross = p1->dx * p0->dy - p0->dx * p1->dy;

            if (cross > 0.0f)
            {

                nleft++;
                p1->flags |= NVG_PT_LEFT;

            }

            limit = maxf(1.01f, minf(p0->len, p1->len) * iw);

            if ((dmr2 * limit * limit) < 1.0f)
                p1->flags |= NVG_PR_INNERBEVEL;

            if (p1->flags & NVG_PT_CORNER)
            {

                if ((dmr2 * miterLimit * miterLimit) < 1.0f || linejoin == NVG_BEVEL || linejoin == NVG_ROUND)
                    p1->flags |= NVG_PT_BEVEL;

            }

            if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
                path->nbevel++;

            p0 = p1++;

        }

        path->convex = (nleft == path->count) ? 1 : 0;

    }

}

int nvg_expand_stroke(struct nvg_context *ctx, float w, float fringe, int linecap, int linejoin, float miterLimit)
{

    struct nvg_pathcache *cache = &ctx->cache;
    struct nvg_vertex *verts = ctx->cache.verts;
    struct nvg_vertex *dst;
    int cverts, i, j;
    float aa = fringe;
    float u0 = 0.0f, u1 = 1.0f;
    int ncap = curveDivs(w, NVG_PI, ctx->tessTol);

    w += aa * 0.5f;

    if (aa == 0.0f)
    {

        u0 = 0.5f;
        u1 = 0.5f;

    }

    calculateJoins(ctx, w, linejoin, miterLimit);

    cverts = 0;

    for (i = 0; i < cache->npaths; i++)
    {

        struct nvg_path *path = &cache->paths[i];
        int loop = (path->closed) ? 1 : 0;

        if (linejoin == NVG_ROUND)
            cverts += (path->count + path->nbevel * (ncap + 2) + 1) * 2;
        else
            cverts += (path->count + path->nbevel * 5 + 1) * 2;

        if (!loop)
        {

            if (linecap == NVG_ROUND)
                cverts += (ncap * 2 + 2) * 2;
            else
                cverts += (3 + 3) * 2;

        }

    }

    for (i = 0; i < cache->npaths; i++)
    {

        struct nvg_path *path = &cache->paths[i];
        struct nvg_point *pts = &cache->points[path->first];
        struct nvg_point *p0;
        struct nvg_point *p1;
        int s, e, loop;
        float dx, dy;

        path->fill = 0;
        path->nfill = 0;
        loop = (path->closed) ? 1 : 0;
        dst = verts;
        path->stroke = dst;

        if (loop)
        {

            p0 = &pts[path->count - 1];
            p1 = &pts[0];
            s = 0;
            e = path->count;

        }

        else
        {

            p0 = &pts[0];
            p1 = &pts[1];
            s = 1;
            e = path->count - 1;

        }

        if (!loop)
        {

            dx = p1->x - p0->x;
            dy = p1->y - p0->y;

            normalize(&dx, &dy);

            if (linecap == NVG_BUTT)
                dst = buttCapStart(dst, p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (linecap == NVG_BUTT || linecap == NVG_SQUARE)
                dst = buttCapStart(dst, p0, dx, dy, w, w-aa, aa, u0, u1);
            else if (linecap == NVG_ROUND)
                dst = roundCapStart(dst, p0, dx, dy, w, ncap, aa, u0, u1);

        }

        for (j = s; j < e; ++j)
        {

            if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
            {

                if (linejoin == NVG_ROUND)
                    dst = roundJoin(dst, p0, p1, w, w, u0, u1, ncap, aa);
                else
                    dst = bevelJoin(dst, p0, p1, w, w, u0, u1, aa);

            }

            else
            {

                vset(dst, p1->x + (p1->dmx * w), p1->y + (p1->dmy * w), u0, 1);

                dst++;

                vset(dst, p1->x - (p1->dmx * w), p1->y - (p1->dmy * w), u1, 1);

                dst++;

            }

            p0 = p1++;

        }

        if (loop)
        {

            vset(dst, verts[0].x, verts[0].y, u0, 1);

            dst++;

            vset(dst, verts[1].x, verts[1].y, u1, 1);

            dst++;

        }

        else
        {

            dx = p1->x - p0->x;
            dy = p1->y - p0->y;

            normalize(&dx, &dy);

            if (linecap == NVG_BUTT)
                dst = buttCapEnd(dst, p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (linecap == NVG_BUTT || linecap == NVG_SQUARE)
                dst = buttCapEnd(dst, p1, dx, dy, w, w-aa, aa, u0, u1);
            else if (linecap == NVG_ROUND)
                dst = roundCapEnd(dst, p1, dx, dy, w, ncap, aa, u0, u1);

        }

        path->nstroke = (int)(dst - verts);
        verts = dst;

    }

    return 1;

}

int nvg_expand_fill(struct nvg_context *ctx, float w, int linejoin, float miterLimit)
{

    struct nvg_pathcache *cache = &ctx->cache;
    struct nvg_vertex *verts = ctx->cache.verts;
    struct nvg_vertex *dst;
    int cverts, convex, i, j;
    int fringe = w > 0.0f;

    calculateJoins(ctx, w, linejoin, miterLimit);

    cverts = 0;

    for (i = 0; i < cache->npaths; i++)
    {

        struct nvg_path *path = &cache->paths[i];
        cverts += path->count + path->nbevel + 1;

        if (fringe)
            cverts += (path->count + path->nbevel * 5 + 1) * 2;

    }

    convex = cache->npaths == 1 && cache->paths[0].convex;

    for (i = 0; i < cache->npaths; i++)
    {

        struct nvg_path *path = &cache->paths[i];
        struct nvg_point *pts = &cache->points[path->first];
        struct nvg_point *p0;
        struct nvg_point *p1;
        float rw, lw, woff;
        float ru, lu;

        woff = 0.5f * ctx->fringeWidth;
        dst = verts;
        path->fill = dst;

        if (fringe)
        {

            p0 = &pts[path->count - 1];
            p1 = &pts[0];

            for (j = 0; j < path->count; ++j)
            {

                if (p1->flags & NVG_PT_BEVEL)
                {

                    float dlx0 = p0->dy;
                    float dly0 = -p0->dx;
                    float dlx1 = p1->dy;
                    float dly1 = -p1->dx;

                    if (p1->flags & NVG_PT_LEFT)
                    {

                        float lx = p1->x + p1->dmx * woff;
                        float ly = p1->y + p1->dmy * woff;

                        vset(dst, lx, ly, 0.5f, 1);

                        dst++;

                    }

                    else
                    {

                        float lx0 = p1->x + dlx0 * woff;
                        float ly0 = p1->y + dly0 * woff;
                        float lx1 = p1->x + dlx1 * woff;
                        float ly1 = p1->y + dly1 * woff;

                        vset(dst, lx0, ly0, 0.5f, 1);

                        dst++;

                        vset(dst, lx1, ly1, 0.5f, 1);

                        dst++;

                    }

                }

                else
                {

                    vset(dst, p1->x + (p1->dmx * woff), p1->y + (p1->dmy * woff), 0.5f, 1);

                    dst++;

                }

                p0 = p1++;

            }

        }

        else
        {

            for (j = 0; j < path->count; ++j)
            {

                vset(dst, pts[j].x, pts[j].y, 0.5f, 1);

                dst++;

            }

        }

        path->nfill = (int)(dst - verts);
        verts = dst;

        if (fringe)
        {

            lw = w + woff;
            rw = w - woff;
            lu = 0;
            ru = 1;
            dst = verts;
            path->stroke = dst;

            if (convex)
            {

                lw = woff;
                lu = 0.5f;

            }

            p0 = &pts[path->count - 1];
            p1 = &pts[0];

            for (j = 0; j < path->count; ++j)
            {

                if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
                {

                    dst = bevelJoin(dst, p0, p1, lw, rw, lu, ru, ctx->fringeWidth);

                }

                else
                {

                    vset(dst, p1->x + (p1->dmx * lw), p1->y + (p1->dmy * lw), lu, 1);

                    dst++;

                    vset(dst, p1->x - (p1->dmx * rw), p1->y - (p1->dmy * rw), ru, 1);

                    dst++;

                }

                p0 = p1++;

            }

            vset(dst, verts[0].x, verts[0].y, lu, 1);

            dst++;

            vset(dst, verts[1].x, verts[1].y, ru, 1);

            dst++;
            path->nstroke = (int)(dst - verts);
            verts = dst;

        }

        else
        {

            path->stroke = NULL;
            path->nstroke = 0;

        }

    }

    return 1;

}

void nvgBeginPath(struct nvg_context *ctx)
{

    ctx->ncommands = 0;

    clearPathCache(ctx);

}

void nvgMoveTo(struct nvg_context *ctx, float x, float y)
{

    float vals[] = { NVG_MOVETO, x, y };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgLineTo(struct nvg_context *ctx, float x, float y)
{

    float vals[] = { NVG_LINETO, x, y };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgBezierTo(struct nvg_context *ctx, float c1x, float c1y, float c2x, float c2y, float x, float y)
{

    float vals[] = { NVG_BEZIERTO, c1x, c1y, c2x, c2y, x, y };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgQuadTo(struct nvg_context *ctx, float cx, float cy, float x, float y)
{

    float x0 = ctx->commandx;
    float y0 = ctx->commandy;
    float vals[] = {
        NVG_BEZIERTO,
        x0 + 2.0f / 3.0f * (cx - x0), y0 + 2.0f / 3.0f * (cy - y0),
        x + 2.0f / 3.0f * (cx - x), y + 2.0f / 3.0f * (cy - y),
        x, y
    };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgArcTo(struct nvg_context *ctx, float x1, float y1, float x2, float y2, float radius)
{

    float x0 = ctx->commandx;
    float y0 = ctx->commandy;
    float dx0, dy0, dx1, dy1, a, d, cx, cy, a0, a1;
    int dir;

    if (!ctx->ncommands)
        return;

    if (ptEquals(x0, y0, x1, y1, ctx->distTol) || ptEquals(x1, y1, x2, y2, ctx->distTol) || distPtSeg(x1, y1, x0, y0, x2, y2) < ctx->distTol * ctx->distTol || radius < ctx->distTol)
    {

        nvgLineTo(ctx, x1, y1);

        return;

    }

    dx0 = x0 - x1;
    dy0 = y0 - y1;
    dx1 = x2 - x1;
    dy1 = y2 - y1;

    normalize(&dx0, &dy0);
    normalize(&dx1, &dy1);
    
    a = acosf(dx0 * dx1 + dy0 * dy1);
    d = radius / tanf(a / 2.0f);

    if (d > 10000.0f)
    {

        nvgLineTo(ctx, x1,y1);

        return;

    }

    if (cross(dx0, dy0, dx1, dy1) > 0.0f)
    {

        cx = x1 + dx0 * d + dy0 * radius;
        cy = y1 + dy0 * d + -dx0 * radius;
        a0 = atan2f(dx0, -dy0);
        a1 = atan2f(-dx1, dy1);
        dir = NVG_CW;

    }

    else
    {

        cx = x1 + dx0 * d + -dy0 * radius;
        cy = y1 + dy0 * d + dx0 * radius;
        a0 = atan2f(-dx0, dy0);
        a1 = atan2f(dx1, -dy1);
        dir = NVG_CCW;

    }

    nvgArc(ctx, cx, cy, radius, a0, a1, dir);

}

void nvgClosePath(struct nvg_context *ctx)
{

    float vals[] = { NVG_CLOSE };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgPathWinding(struct nvg_context *ctx, int dir)
{

    float vals[] = { NVG_WINDING, (float)dir };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgArc(struct nvg_context *ctx, float cx, float cy, float r, float a0, float a1, int dir)
{

    float a = 0, da = 0, hda = 0, kappa = 0;
    float dx = 0, dy = 0, x = 0, y = 0, tanx = 0, tany = 0;
    float px = 0, py = 0, ptanx = 0, ptany = 0;
    float vals[3 + 5 * 7 + 100];
    int i, ndivs, nvals;
    int move = ctx->ncommands > 0 ? NVG_LINETO : NVG_MOVETO;

    da = a1 - a0;

    if (dir == NVG_CW)
    {

        if (absf(da) >= NVG_PI * 2)
        {

            da = NVG_PI * 2;

        }

        else
        {

            while (da < 0.0f)
                da += NVG_PI * 2;

        }

    }

    else
    {

        if (absf(da) >= NVG_PI * 2)
        {

            da = -NVG_PI * 2;

        }

        else
        {

            while (da > 0.0f)
                da -= NVG_PI * 2;

        }

    }

    ndivs = maxi(1, mini((int)(absf(da) / (NVG_PI * 0.5f) + 0.5f), 5));
    hda = (da / (float)ndivs) / 2.0f;
    kappa = absf(4.0f / 3.0f * (1.0f - cosf(hda)) / sinf(hda));

    if (dir == NVG_CCW)
        kappa = -kappa;

    nvals = 0;

    for (i = 0; i <= ndivs; i++)
    {

        a = a0 + da * (i / (float)ndivs);
        dx = cosf(a);
        dy = sinf(a);
        x = cx + dx * r;
        y = cy + dy * r;
        tanx = -dy * r * kappa;
        tany = dx * r * kappa;

        if (i == 0)
        {

            vals[nvals++] = (float)move;
            vals[nvals++] = x;
            vals[nvals++] = y;

        }

        else
        {

            vals[nvals++] = NVG_BEZIERTO;
            vals[nvals++] = px + ptanx;
            vals[nvals++] = py + ptany;
            vals[nvals++] = x - tanx;
            vals[nvals++] = y - tany;
            vals[nvals++] = x;
            vals[nvals++] = y;

        }

        px = x;
        py = y;
        ptanx = tanx;
        ptany = tany;

    }

    appendCommands(ctx, vals, nvals);

}

void nvgRect(struct nvg_context *ctx, float x, float y, float w, float h)
{

    float vals[] = {
        NVG_MOVETO, x, y,
        NVG_LINETO, x, y + h,
        NVG_LINETO, x + w, y + h,
        NVG_LINETO, x + w, y,
        NVG_CLOSE
    };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgRoundedRect(struct nvg_context *ctx, float x, float y, float w, float h, float r)
{

    nvgRoundedRectVarying(ctx, x, y, w, h, r, r, r, r);

}

void nvgRoundedRectVarying(struct nvg_context *ctx, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft)
{

    if (radTopLeft < 0.1f && radTopRight < 0.1f && radBottomRight < 0.1f && radBottomLeft < 0.1f)
    {

        nvgRect(ctx, x, y, w, h);

        return;

    }

    else
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
            NVG_BEZIERTO, x, y + h - ryBL * (1 - NVG_KAPPA90), x + rxBL*(1 - NVG_KAPPA90), y + h, x + rxBL, y + h,
            NVG_LINETO, x + w - rxBR, y + h,
            NVG_BEZIERTO, x + w - rxBR * (1 - NVG_KAPPA90), y + h, x + w, y + h - ryBR * (1 - NVG_KAPPA90), x + w, y + h - ryBR,
            NVG_LINETO, x + w, y + ryTR,
            NVG_BEZIERTO, x + w, y + ryTR * (1 - NVG_KAPPA90), x + w - rxTR * (1 - NVG_KAPPA90), y, x + w - rxTR, y,
            NVG_LINETO, x + rxTL, y,
            NVG_BEZIERTO, x + rxTL * (1 - NVG_KAPPA90), y, x, y + ryTL * (1 - NVG_KAPPA90), x, y + ryTL,
            NVG_CLOSE
        };

        appendCommands(ctx, vals, NVG_COUNTOF(vals));

    }

}

void nvgEllipse(struct nvg_context *ctx, float cx, float cy, float rx, float ry)
{

    float vals[] = {
        NVG_MOVETO, cx - rx, cy,
        NVG_BEZIERTO, cx - rx, cy + ry * NVG_KAPPA90, cx - rx * NVG_KAPPA90, cy + ry, cx, cy + ry,
        NVG_BEZIERTO, cx + rx * NVG_KAPPA90, cy + ry, cx + rx, cy + ry * NVG_KAPPA90, cx + rx, cy,
        NVG_BEZIERTO, cx + rx, cy - ry * NVG_KAPPA90, cx + rx * NVG_KAPPA90, cy - ry, cx, cy - ry,
        NVG_BEZIERTO, cx - rx * NVG_KAPPA90, cy - ry, cx - rx, cy - ry * NVG_KAPPA90, cx - rx, cy,
        NVG_CLOSE
    };

    appendCommands(ctx, vals, NVG_COUNTOF(vals));

}

void nvgCircle(struct nvg_context *ctx, float cx, float cy, float r)
{

    nvgEllipse(ctx, cx, cy, r, r);

}

void nvg_init(struct nvg_context *ctx)
{

    setPaintColor(&ctx->state.fill, nvgRGBA(255, 255, 255, 255));
    setPaintColor(&ctx->state.stroke, nvgRGBA(0, 0, 0, 255));

    ctx->state.shapeAntiAlias = 1;
    ctx->state.strokeWidth = 1.0f;
    ctx->state.miterLimit = 10.0f;
    ctx->state.linecap = NVG_BUTT;
    ctx->state.linejoin = NVG_MITER;

    nvgTransformIdentity(ctx->state.xform);

    ctx->state.scissor.extent[0] = -1.0f;
    ctx->state.scissor.extent[1] = -1.0f;

}

