#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "nvg.h"
#define FONTSTASH_IMPLEMENTATION
#include "fontstash.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NVG_INIT_FONTIMAGE_SIZE 512
#define NVG_MAX_FONTIMAGE_SIZE 2048
#define NVG_INIT_COMMANDS_SIZE 256
#define NVG_INIT_POINTS_SIZE 128
#define NVG_INIT_PATHS_SIZE 16
#define NVG_INIT_VERTS_SIZE 256
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

static void deletePathCache(struct nvg_pathcache *c)
{

    if (c == NULL)
        return;

    if (c->points != NULL)
        free(c->points);

    if (c->paths != NULL)
        free(c->paths);

    if (c->verts != NULL)
        free(c->verts);

    free(c);

}

static struct nvg_pathcache *allocPathCache(void)
{

    struct nvg_pathcache *c = malloc(sizeof (struct nvg_pathcache));

    if (c == NULL)
        goto error;

    memset(c, 0, sizeof (struct nvg_pathcache));

    c->points = malloc(sizeof (struct nvg_point) * NVG_INIT_POINTS_SIZE);

    if (!c->points)
        goto error;

    c->npoints = 0;
    c->cpoints = NVG_INIT_POINTS_SIZE;
    c->paths = malloc(sizeof (struct nvg_path) * NVG_INIT_PATHS_SIZE);

    if (!c->paths)
        goto error;

    c->npaths = 0;
    c->cpaths = NVG_INIT_PATHS_SIZE;
    c->verts = malloc(sizeof (struct nvg_vertex) * NVG_INIT_VERTS_SIZE);

    if (!c->verts)
        goto error;

    c->nverts = 0;
    c->cverts = NVG_INIT_VERTS_SIZE;

    return c;

error:
    deletePathCache(c);

    return NULL;

}

static void setDevicePixelRatio(struct nvg_context *ctx, float ratio)
{

    ctx->tessTol = 0.25f / ratio;
    ctx->distTol = 0.01f / ratio;
    ctx->fringeWidth = 1.0f / ratio;
    ctx->devicePxRatio = ratio;

}

static struct nvg_compositeoperationstate compositeoperationstate(int op)
{

    struct nvg_compositeoperationstate state;
    int sfactor, dfactor;

    if (op == NVG_SOURCE_OVER)
    {

        sfactor = NVG_ONE;
        dfactor = NVG_ONE_MINUS_SRC_ALPHA;

    }

    else if (op == NVG_SOURCE_IN)
    {

        sfactor = NVG_DST_ALPHA;
        dfactor = NVG_ZERO;

    }

    else if (op == NVG_SOURCE_OUT)
    {

        sfactor = NVG_ONE_MINUS_DST_ALPHA;
        dfactor = NVG_ZERO;

    }

    else if (op == NVG_ATOP)
    {

        sfactor = NVG_DST_ALPHA;
        dfactor = NVG_ONE_MINUS_SRC_ALPHA;

    }

    else if (op == NVG_DESTINATION_OVER)
    {

        sfactor = NVG_ONE_MINUS_DST_ALPHA;
        dfactor = NVG_ONE;

    }

    else if (op == NVG_DESTINATION_IN)
    {

        sfactor = NVG_ZERO;
        dfactor = NVG_SRC_ALPHA;

    }

    else if (op == NVG_DESTINATION_OUT)
    {

        sfactor = NVG_ZERO;
        dfactor = NVG_ONE_MINUS_SRC_ALPHA;

    }

    else if (op == NVG_DESTINATION_ATOP)
    {

        sfactor = NVG_ONE_MINUS_DST_ALPHA;
        dfactor = NVG_SRC_ALPHA;

    }

    else if (op == NVG_LIGHTER)
    {

        sfactor = NVG_ONE;
        dfactor = NVG_ONE;

    }

    else if (op == NVG_COPY)
    {

        sfactor = NVG_ONE;
        dfactor = NVG_ZERO;

    }

    else if (op == NVG_XOR)
    {

        sfactor = NVG_ONE_MINUS_DST_ALPHA;
        dfactor = NVG_ONE_MINUS_SRC_ALPHA;

    }

    else
    {

        sfactor = NVG_ONE;
        dfactor = NVG_ZERO;

    }

    state.srcRGB = sfactor;
    state.dstRGB = dfactor;
    state.srcAlpha = sfactor;
    state.dstAlpha = dfactor;

    return state;

}

static struct nvg_state *getState(struct nvg_context *ctx)
{

    return &ctx->states[ctx->nstates - 1];

}

struct nvg_context *nvgCreateInternal(struct nvg_params *params)
{

    FONSparams fontParams;
    struct nvg_context *ctx = malloc(sizeof (struct nvg_context));
    int i;

    if (ctx == NULL)
        goto error;

    memset(ctx, 0, sizeof (struct nvg_context));

    ctx->params = *params;

    for (i = 0; i < NVG_MAX_FONTIMAGES; i++)
        ctx->fontImages[i] = 0;

    ctx->commands = malloc(sizeof (float) * NVG_INIT_COMMANDS_SIZE);

    if (!ctx->commands)
        goto error;

    ctx->ncommands = 0;
    ctx->ccommands = NVG_INIT_COMMANDS_SIZE;
    ctx->cache = allocPathCache();

    if (ctx->cache == NULL)
        goto error;

    nvgSave(ctx);
    nvgReset(ctx);
    setDevicePixelRatio(ctx, 1.0f);

    if (ctx->params.renderCreate(ctx->params.userPtr) == 0)
        goto error;

    memset(&fontParams, 0, sizeof (fontParams));

    fontParams.width = NVG_INIT_FONTIMAGE_SIZE;
    fontParams.height = NVG_INIT_FONTIMAGE_SIZE;
    fontParams.flags = FONS_ZERO_TOPLEFT;
    fontParams.renderCreate = NULL;
    fontParams.renderUpdate = NULL;
    fontParams.renderDraw = NULL;
    fontParams.renderDelete = NULL;
    fontParams.userPtr = NULL;
    ctx->fs = fonsCreateInternal(&fontParams);

    if (ctx->fs == NULL)
        goto error;

    ctx->fontImages[0] = ctx->params.renderCreateTexture(ctx->params.userPtr, NVG_TEXTURE_ALPHA, fontParams.width, fontParams.height, 0, NULL);

    if (ctx->fontImages[0] == 0)
        goto error;

    ctx->fontImageIdx = 0;

    return ctx;

error:
    nvgDeleteInternal(ctx);

    return 0;

}

void nvgDeleteInternal(struct nvg_context *ctx)
{

    int i;

    if (ctx == NULL)
        return;

    if (ctx->commands != NULL)
        free(ctx->commands);

    if (ctx->cache != NULL)
        deletePathCache(ctx->cache);

    if (ctx->fs)
        fonsDeleteInternal(ctx->fs);

    for (i = 0; i < NVG_MAX_FONTIMAGES; i++)
    {

        if (ctx->fontImages[i] != 0)
        {

            nvgDeleteImage(ctx, ctx->fontImages[i]);

            ctx->fontImages[i] = 0;

        }

    }

    if (ctx->params.renderDelete != NULL)
        ctx->params.renderDelete(ctx->params.userPtr);

    free(ctx);

}

void nvgBeginFrame(struct nvg_context *ctx, float windowWidth, float windowHeight, float devicePixelRatio)
{

    ctx->nstates = 0;

    nvgSave(ctx);
    nvgReset(ctx);

    setDevicePixelRatio(ctx, devicePixelRatio);
    ctx->params.renderViewport(ctx->params.userPtr, windowWidth, windowHeight, devicePixelRatio);

    ctx->drawCallCount = 0;
    ctx->fillTriCount = 0;
    ctx->strokeTriCount = 0;
    ctx->textTriCount = 0;

}

void nvgCancelFrame(struct nvg_context *ctx)
{

    ctx->params.renderCancel(ctx->params.userPtr);

}

void nvgEndFrame(struct nvg_context *ctx)
{

    ctx->params.renderFlush(ctx->params.userPtr);

    if (ctx->fontImageIdx != 0)
    {

        int fontImage = ctx->fontImages[ctx->fontImageIdx];
        int i, j, iw, ih;

        if (fontImage == 0)
            return;

        nvgImageSize(ctx, fontImage, &iw, &ih);

        for (i = j = 0; i < ctx->fontImageIdx; i++)
        {

            if (ctx->fontImages[i] != 0)
            {

                int nw, nh;

                nvgImageSize(ctx, ctx->fontImages[i], &nw, &nh);

                if (nw < iw || nh < ih)
                    nvgDeleteImage(ctx, ctx->fontImages[i]);
                else
                    ctx->fontImages[j++] = ctx->fontImages[i];

            }

        }

        ctx->fontImages[j++] = ctx->fontImages[0];
        ctx->fontImages[0] = fontImage;
        ctx->fontImageIdx = 0;

        for (i = j; i < NVG_MAX_FONTIMAGES; i++)
            ctx->fontImages[i] = 0;
    }

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

    int i;
    float oneminu;
    struct nvg_color cint = {{{0}}};

    u = clampf(u, 0.0f, 1.0f);
    oneminu = 1.0f - u;

    for (i = 0; i < 4; i++)
        cint.rgba[i] = c0.rgba[i] * oneminu + c1.rgba[i] * u;

    return cint;

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

    *dx = sx*t[0] + sy*t[2] + t[4];
    *dy = sx*t[1] + sy*t[3] + t[5];

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

void nvgSave(struct nvg_context *ctx)
{

    if (ctx->nstates >= NVG_MAX_STATES)
        return;

    if (ctx->nstates > 0)
        memcpy(&ctx->states[ctx->nstates], &ctx->states[ctx->nstates - 1], sizeof (struct nvg_state));

    ctx->nstates++;

}

void nvgRestore(struct nvg_context *ctx)
{

    if (ctx->nstates <= 1)
        return;

    ctx->nstates--;

}

void nvgReset(struct nvg_context *ctx)
{

    struct nvg_state *state = getState(ctx);

    memset(state, 0, sizeof (struct nvg_state));

    setPaintColor(&state->fill, nvgRGBA(255, 255, 255, 255));
    setPaintColor(&state->stroke, nvgRGBA(0, 0, 0, 255));
    state->compositeoperation = compositeoperationstate(NVG_SOURCE_OVER);
    state->shapeAntiAlias = 1;
    state->strokeWidth = 1.0f;
    state->miterLimit = 10.0f;
    state->linecap = NVG_BUTT;
    state->linejoin = NVG_MITER;

    nvgTransformIdentity(state->xform);

    state->scissor.extent[0] = -1.0f;
    state->scissor.extent[1] = -1.0f;
    state->fontSize = 16.0f;
    state->letterSpacing = 0.0f;
    state->fontBlur = 0.0f;
    state->textAlign = NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE;
    state->fontId = 0;

}

void nvgShapeAntiAlias(struct nvg_context *ctx, int enabled)
{

    struct nvg_state *state = getState(ctx);

    state->shapeAntiAlias = enabled;

}

void nvgStrokeWidth(struct nvg_context *ctx, float width)
{

    struct nvg_state *state = getState(ctx);

    state->strokeWidth = width;

}

void nvgMiterLimit(struct nvg_context *ctx, float limit)
{

    struct nvg_state *state = getState(ctx);

    state->miterLimit = limit;

}

void nvgLineCap(struct nvg_context *ctx, int cap)
{

    struct nvg_state *state = getState(ctx);

    state->linecap = cap;

}

void nvgLineJoin(struct nvg_context *ctx, int join)
{

    struct nvg_state *state = getState(ctx);

    state->linejoin = join;

}

void nvgTransform(struct nvg_context *ctx, float a, float b, float c, float d, float e, float f)
{

    struct nvg_state *state = getState(ctx);
    float t[6] = { a, b, c, d, e, f };

    nvgTransformPremultiply(state->xform, t);

}

void nvgResetTransform(struct nvg_context *ctx)
{

    struct nvg_state *state = getState(ctx);

    nvgTransformIdentity(state->xform);

}

void nvgTranslate(struct nvg_context *ctx, float x, float y)
{

    struct nvg_state *state = getState(ctx);
    float t[6];

    nvgTransformTranslate(t, x,y);
    nvgTransformPremultiply(state->xform, t);

}

void nvgRotate(struct nvg_context *ctx, float angle)
{

    struct nvg_state *state = getState(ctx);
    float t[6];

    nvgTransformRotate(t, angle);
    nvgTransformPremultiply(state->xform, t);

}

void nvgSkewX(struct nvg_context *ctx, float angle)
{

    struct nvg_state *state = getState(ctx);
    float t[6];

    nvgTransformSkewX(t, angle);
    nvgTransformPremultiply(state->xform, t);

}

void nvgSkewY(struct nvg_context *ctx, float angle)
{

    struct nvg_state *state = getState(ctx);
    float t[6];

    nvgTransformSkewY(t, angle);
    nvgTransformPremultiply(state->xform, t);

}

void nvgScale(struct nvg_context *ctx, float x, float y)
{

    struct nvg_state *state = getState(ctx);
    float t[6];

    nvgTransformScale(t, x,y);
    nvgTransformPremultiply(state->xform, t);

}

void nvgCurrentTransform(struct nvg_context *ctx, float *xform)
{

    struct nvg_state *state = getState(ctx);

    if (xform == NULL)
        return;

    memcpy(xform, state->xform, sizeof (float) * 6);

}

void nvgStrokeColor(struct nvg_context *ctx, struct nvg_color color)
{

    struct nvg_state *state = getState(ctx);

    setPaintColor(&state->stroke, color);

}

void nvgStrokePaint(struct nvg_context *ctx, struct nvg_paint paint)
{

    struct nvg_state *state = getState(ctx);

    state->stroke = paint;

    nvgTransformMultiply(state->stroke.xform, state->xform);

}

void nvgFillColor(struct nvg_context *ctx, struct nvg_color color)
{

    struct nvg_state *state = getState(ctx);

    setPaintColor(&state->fill, color);

}

void nvgFillPaint(struct nvg_context *ctx, struct nvg_paint paint)
{

    struct nvg_state *state = getState(ctx);

    state->fill = paint;

    nvgTransformMultiply(state->fill.xform, state->xform);

}

int nvgCreateImage(struct nvg_context *ctx, const char *filename, int imageFlags)
{

    int w, h, n, image;
    unsigned char *img;

    stbi_set_unpremultiply_on_load(1);
    stbi_convert_iphone_png_to_rgb(1);

    img = stbi_load(filename, &w, &h, &n, 4);

    if (img == NULL)
        return 0;

    image = nvgCreateImageRGBA(ctx, w, h, imageFlags, img);

    stbi_image_free(img);

    return image;

}

int nvgCreateImageMem(struct nvg_context *ctx, int imageFlags, unsigned char *data, int ndata)
{

    int w, h, n, image;
    unsigned char *img = stbi_load_from_memory(data, ndata, &w, &h, &n, 4);

    if (img == NULL)
        return 0;

    image = nvgCreateImageRGBA(ctx, w, h, imageFlags, img);

    stbi_image_free(img);

    return image;

}

int nvgCreateImageRGBA(struct nvg_context *ctx, int w, int h, int imageFlags, const unsigned char *data)
{

    return ctx->params.renderCreateTexture(ctx->params.userPtr, NVG_TEXTURE_RGBA, w, h, imageFlags, data);

}

void nvgUpdateImage(struct nvg_context *ctx, int image, const unsigned char *data)
{

    int w, h;

    ctx->params.renderGetTextureSize(ctx->params.userPtr, image, &w, &h);
    ctx->params.renderUpdateTexture(ctx->params.userPtr, image, 0,0, w,h, data);
}

void nvgImageSize(struct nvg_context *ctx, int image, int *w, int *h)
{

    ctx->params.renderGetTextureSize(ctx->params.userPtr, image, w, h);

}

void nvgDeleteImage(struct nvg_context *ctx, int image)
{

    ctx->params.renderDeleteTexture(ctx->params.userPtr, image);

}

struct nvg_paint nvgLinearGradient(struct nvg_context *ctx, float sx, float sy, float ex, float ey, struct nvg_color icol, struct nvg_color ocol)
{

    struct nvg_paint p;
    float dx, dy, d;
    const float large = 1e5;

    memset(&p, 0, sizeof (struct nvg_paint));

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

    p.xform[0] = dy;
    p.xform[1] = -dx;
    p.xform[2] = dx;
    p.xform[3] = dy;
    p.xform[4] = sx - dx * large;
    p.xform[5] = sy - dy * large;
    p.extent[0] = large;
    p.extent[1] = large + d * 0.5f;
    p.radius = 0.0f;
    p.feather = maxf(1.0f, d);
    p.innerColor = icol;
    p.outerColor = ocol;

    return p;

}

struct nvg_paint nvgRadialGradient(struct nvg_context *ctx, float cx, float cy, float inr, float outr, struct nvg_color icol, struct nvg_color ocol)
{

    struct nvg_paint p;
    float r = (inr + outr) * 0.5f;
    float f = (outr - inr);

    memset(&p, 0, sizeof (struct nvg_paint));
    nvgTransformIdentity(p.xform);

    p.xform[4] = cx;
    p.xform[5] = cy;
    p.extent[0] = r;
    p.extent[1] = r;
    p.radius = r;
    p.feather = maxf(1.0f, f);
    p.innerColor = icol;
    p.outerColor = ocol;

    return p;

}

struct nvg_paint nvgBoxGradient(struct nvg_context *ctx, float x, float y, float w, float h, float r, float f, struct nvg_color icol, struct nvg_color ocol)
{

    struct nvg_paint p;

    memset(&p, 0, sizeof (struct nvg_paint));
    nvgTransformIdentity(p.xform);

    p.xform[4] = x + w * 0.5f;
    p.xform[5] = y + h * 0.5f;
    p.extent[0] = w * 0.5f;
    p.extent[1] = h * 0.5f;
    p.radius = r;
    p.feather = maxf(1.0f, f);
    p.innerColor = icol;
    p.outerColor = ocol;

    return p;

}

struct nvg_paint nvgImagePattern(struct nvg_context *ctx, float cx, float cy, float w, float h, float angle, int image, float alpha)
{

    struct nvg_paint p;

    memset(&p, 0, sizeof (struct nvg_paint));
    nvgTransformRotate(p.xform, angle);

    p.xform[4] = cx;
    p.xform[5] = cy;
    p.extent[0] = w;
    p.extent[1] = h;
    p.image = image;
    p.innerColor = p.outerColor = nvgRGBAf(1, 1, 1, alpha);

    return p;

}

void nvgScissor(struct nvg_context *ctx, float x, float y, float w, float h)
{

    struct nvg_state *state = getState(ctx);

    w = maxf(0.0f, w);
    h = maxf(0.0f, h);

    nvgTransformIdentity(state->scissor.xform);

    state->scissor.xform[4] = x + w * 0.5f;
    state->scissor.xform[5] = y + h * 0.5f;

    nvgTransformMultiply(state->scissor.xform, state->xform);

    state->scissor.extent[0] = w * 0.5f;
    state->scissor.extent[1] = h * 0.5f;

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

    struct nvg_state *state = getState(ctx);
    float pxform[6], invxorm[6];
    float rect[4];
    float ex, ey, tex, tey;

    if (state->scissor.extent[0] < 0)
    {

        nvgScissor(ctx, x, y, w, h);

        return;

    }

    memcpy(pxform, state->scissor.xform, sizeof (float) * 6);

    ex = state->scissor.extent[0];
    ey = state->scissor.extent[1];

    nvgTransformInverse(invxorm, state->xform);
    nvgTransformMultiply(pxform, invxorm);

    tex = ex*absf(pxform[0]) + ey * absf(pxform[2]);
    tey = ex*absf(pxform[1]) + ey * absf(pxform[3]);

    isectRects(rect, pxform[4] - tex, pxform[5] - tey, tex * 2, tey * 2, x, y, w, h);
    nvgScissor(ctx, rect[0], rect[1], rect[2], rect[3]);

}

void nvgResetScissor(struct nvg_context *ctx)
{

    struct nvg_state *state = getState(ctx);

    memset(state->scissor.xform, 0, sizeof (state->scissor.xform));

    state->scissor.extent[0] = -1.0f;
    state->scissor.extent[1] = -1.0f;

}

void nvgGlobalCompositeOperation(struct nvg_context *ctx, int op)
{

    struct nvg_state *state = getState(ctx);

    state->compositeoperation = compositeoperationstate(op);

}

void nvgGlobalCompositeBlendFunc(struct nvg_context *ctx, int sfactor, int dfactor)
{

    nvgGlobalCompositeBlendFuncSeparate(ctx, sfactor, dfactor, sfactor, dfactor);

}

void nvgGlobalCompositeBlendFuncSeparate(struct nvg_context *ctx, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha)
{

    struct nvg_state *state = getState(ctx);
    struct nvg_compositeoperationstate op;

    op.srcRGB = srcRGB;
    op.dstRGB = dstRGB;
    op.srcAlpha = srcAlpha;
    op.dstAlpha = dstAlpha;

    state->compositeoperation = op;

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

    struct nvg_state *state = getState(ctx);
    int i;

    if (ctx->ncommands + nvals > ctx->ccommands)
    {

        int ccommands = ctx->ncommands + nvals + ctx->ccommands / 2;
        float *commands = realloc(ctx->commands, sizeof (float) * ccommands);

        if (commands == NULL)
            return;

        ctx->commands = commands;
        ctx->ccommands = ccommands;

    }

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
            nvgTransformPoint(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1], vals[i + 2]);

            i += 3;

            break;

        case NVG_LINETO:
            nvgTransformPoint(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],vals[i + 2]);

            i += 3;

            break;

        case NVG_BEZIERTO:
            nvgTransformPoint(&vals[i + 1],&vals[i + 2], state->xform, vals[i + 1], vals[i + 2]);
            nvgTransformPoint(&vals[i + 3],&vals[i + 4], state->xform, vals[i + 3], vals[i + 4]);
            nvgTransformPoint(&vals[i + 5],&vals[i + 6], state->xform, vals[i + 5], vals[i + 6]);

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

    ctx->cache->npoints = 0;
    ctx->cache->npaths = 0;

}

static struct nvg_path *lastPath(struct nvg_context *ctx)
{

    if (ctx->cache->npaths > 0)
        return &ctx->cache->paths[ctx->cache->npaths - 1];

    return NULL;

}

static void addPath(struct nvg_context *ctx)
{

    struct nvg_path *path;

    if (ctx->cache->npaths + 1 > ctx->cache->cpaths)
    {

        int cpaths = ctx->cache->npaths + 1 + ctx->cache->cpaths / 2;
        struct nvg_path *paths = realloc(ctx->cache->paths, sizeof (struct nvg_path) * cpaths);

        if (paths == NULL)
            return;

        ctx->cache->paths = paths;
        ctx->cache->cpaths = cpaths;

    }

    path = &ctx->cache->paths[ctx->cache->npaths];

    memset(path, 0, sizeof (struct nvg_path));

    path->first = ctx->cache->npoints;
    path->winding = NVG_CCW;
    ctx->cache->npaths++;

}

static struct nvg_point *lastPoint(struct nvg_context *ctx)
{

    if (ctx->cache->npoints > 0)
        return &ctx->cache->points[ctx->cache->npoints - 1];

    return NULL;

}

static void addPoint(struct nvg_context *ctx, float x, float y, int flags)
{

    struct nvg_path *path = lastPath(ctx);
    struct nvg_point *pt;

    if (path == NULL)
        return;

    if (path->count > 0 && ctx->cache->npoints > 0)
    {

        pt = lastPoint(ctx);

        if (ptEquals(pt->x,pt->y, x,y, ctx->distTol))
        {

            pt->flags |= flags;

            return;

        }

    }

    if (ctx->cache->npoints + 1 > ctx->cache->cpoints)
    {

        int cpoints = ctx->cache->npoints + 1 + ctx->cache->cpoints / 2;
        struct nvg_point *points = realloc(ctx->cache->points, sizeof (struct nvg_point) * cpoints);

        if (points == NULL)
            return;

        ctx->cache->points = points;
        ctx->cache->cpoints = cpoints;

    }

    pt = &ctx->cache->points[ctx->cache->npoints];

    memset(pt, 0, sizeof (struct nvg_point));

    pt->x = x;
    pt->y = y;
    pt->flags = (unsigned char)flags;
    ctx->cache->npoints++;
    path->count++;

}

static void closePath(struct nvg_context *ctx)
{

    struct nvg_path *path = lastPath(ctx);

    if (path == NULL)
        return;

    path->closed = 1;

}

static void pathWinding(struct nvg_context *ctx, int winding)
{

    struct nvg_path *path = lastPath(ctx);

    if (path == NULL)
        return;

    path->winding = winding;

}

static float getAverageScale(float *t)
{

    float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
    float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);

    return (sx + sy) * 0.5f;

}

static struct nvg_vertex *allocTempVerts(struct nvg_context *ctx, int nverts)
{

    if (nverts > ctx->cache->cverts)
    {

        int cverts = (nverts + 0xff) & ~0xff;
        struct nvg_vertex *verts = realloc(ctx->cache->verts, sizeof (struct nvg_vertex) * cverts);

        if (verts == NULL)
            return NULL;

        ctx->cache->verts = verts;
        ctx->cache->cverts = cverts;

    }

    return ctx->cache->verts;

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

static void flattenPaths(struct nvg_context *ctx)
{

    struct nvg_pathcache *cache = ctx->cache;
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

            if (last != NULL)
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

    vset(dst, px + dlx*w, py + dly * w, u0, 1);

    dst++;

    vset(dst, px - dlx*w, py - dly * w, u1, 1);

    dst++;

    for (i = 0; i < ncap; i++)
    {

        float a = i/(float)(ncap - 1) * NVG_PI;
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

    struct nvg_pathcache *cache = ctx->cache;
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

static int expandStroke(struct nvg_context *ctx, float w, float fringe, int linecap, int linejoin, float miterLimit)
{

    struct nvg_pathcache *cache = ctx->cache;
    struct nvg_vertex *verts;
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
        int loop = (path->closed == 0) ? 0 : 1;

        if (linejoin == NVG_ROUND)
            cverts += (path->count + path->nbevel * (ncap + 2) + 1) * 2;
        else
            cverts += (path->count + path->nbevel * 5 + 1) * 2;

        if (loop == 0)
        {

            if (linecap == NVG_ROUND)
                cverts += (ncap * 2 + 2) * 2;
            else
                cverts += (3 + 3) * 2;

        }

    }

    verts = allocTempVerts(ctx, cverts);

    if (verts == NULL)
        return 0;

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
        loop = (path->closed == 0) ? 0 : 1;
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

        if (loop == 0)
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

static int expandFill(struct nvg_context *ctx, float w, int linejoin, float miterLimit)
{

    struct nvg_pathcache *cache = ctx->cache;
    struct nvg_vertex *verts;
    struct nvg_vertex *dst;
    int cverts, convex, i, j;
    float aa = ctx->fringeWidth;
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

    verts = allocTempVerts(ctx, cverts);

    if (verts == NULL)
        return 0;

    convex = cache->npaths == 1 && cache->paths[0].convex;

    for (i = 0; i < cache->npaths; i++)
    {

        struct nvg_path *path = &cache->paths[i];
        struct nvg_point *pts = &cache->points[path->first];
        struct nvg_point *p0;
        struct nvg_point *p1;
        float rw, lw, woff;
        float ru, lu;

        woff = 0.5f * aa;
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

    if (ctx->ncommands == 0)
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

void nvgFill(struct nvg_context *ctx)
{

    struct nvg_state *state = getState(ctx);
    const struct nvg_path *path;
    struct nvg_paint fillPaint = state->fill;
    int i;

    flattenPaths(ctx);

    if (ctx->params.edgeAntiAlias && state->shapeAntiAlias)
        expandFill(ctx, ctx->fringeWidth, NVG_MITER, 2.4f);
    else
        expandFill(ctx, 0.0f, NVG_MITER, 2.4f);

    ctx->params.renderFill(ctx->params.userPtr, &fillPaint, state->compositeoperation, &state->scissor, ctx->fringeWidth, ctx->cache->bounds, ctx->cache->paths, ctx->cache->npaths);

    for (i = 0; i < ctx->cache->npaths; i++)
    {

        path = &ctx->cache->paths[i];
        ctx->fillTriCount += path->nfill - 2;
        ctx->fillTriCount += path->nstroke - 2;
        ctx->drawCallCount += 2;

    }

}

void nvgStroke(struct nvg_context *ctx)
{

    struct nvg_state *state = getState(ctx);
    float scale = getAverageScale(state->xform);
    float strokeWidth = clampf(state->strokeWidth * scale, 0.0f, 200.0f);
    struct nvg_paint strokePaint = state->stroke;
    const struct nvg_path *path;
    int i;

    if (strokeWidth < ctx->fringeWidth)
    {

        float alpha = clampf(strokeWidth / ctx->fringeWidth, 0.0f, 1.0f);

        strokePaint.innerColor.a *= alpha * alpha;
        strokePaint.outerColor.a *= alpha * alpha;
        strokeWidth = ctx->fringeWidth;

    }

    flattenPaths(ctx);

    if (ctx->params.edgeAntiAlias && state->shapeAntiAlias)
        expandStroke(ctx, strokeWidth * 0.5f, ctx->fringeWidth, state->linecap, state->linejoin, state->miterLimit);
    else
        expandStroke(ctx, strokeWidth * 0.5f, 0.0f, state->linecap, state->linejoin, state->miterLimit);

    ctx->params.renderStroke(ctx->params.userPtr, &strokePaint, state->compositeoperation, &state->scissor, ctx->fringeWidth, strokeWidth, ctx->cache->paths, ctx->cache->npaths);

    for (i = 0; i < ctx->cache->npaths; i++)
    {

        path = &ctx->cache->paths[i];
        ctx->strokeTriCount += path->nstroke - 2;
        ctx->drawCallCount++;

    }
    
}

int nvgCreateFont(struct nvg_context *ctx, const char *name, const char *path)
{

    return fonsAddFont(ctx->fs, name, path);

}

int nvgCreateFontMem(struct nvg_context *ctx, const char *name, unsigned char *data, int ndata, int freeData)
{

    return fonsAddFontMem(ctx->fs, name, data, ndata, freeData);

}

void nvgFontSize(struct nvg_context *ctx, float size)
{

    struct nvg_state *state = getState(ctx);

    state->fontSize = size;

}

void nvgFontBlur(struct nvg_context *ctx, float blur)
{

    struct nvg_state *state = getState(ctx);

    state->fontBlur = blur;

}

void nvgTextLetterSpacing(struct nvg_context *ctx, float spacing)
{

    struct nvg_state *state = getState(ctx);

    state->letterSpacing = spacing;

}

void nvgTextAlign(struct nvg_context *ctx, int align)
{

    struct nvg_state *state = getState(ctx);

    state->textAlign = align;

}

void nvgFontFaceId(struct nvg_context *ctx, int font)
{

    struct nvg_state *state = getState(ctx);

    state->fontId = font;

}

static float quantize(float a, float d)
{

    return ((int)(a / d + 0.5f)) * d;

}

static float getFontScale(struct nvg_state *state)
{

    return minf(quantize(getAverageScale(state->xform), 0.01f), 4.0f);

}

static void flushTextTexture(struct nvg_context *ctx)
{

    int dirty[4];

    if (fonsValidateTexture(ctx->fs, dirty))
    {

        int fontImage = ctx->fontImages[ctx->fontImageIdx];

        if (fontImage != 0)
        {

            int iw, ih;
            const unsigned char *data = fonsGetTextureData(ctx->fs, &iw, &ih);
            int x = dirty[0];
            int y = dirty[1];
            int w = dirty[2] - dirty[0];
            int h = dirty[3] - dirty[1];

            ctx->params.renderUpdateTexture(ctx->params.userPtr, fontImage, x, y, w, h, data);

        }

    }

}

static int allocTextAtlas(struct nvg_context *ctx)
{

    int iw, ih;

    flushTextTexture(ctx);

    if (ctx->fontImageIdx >= NVG_MAX_FONTIMAGES - 1)
        return 0;

    if (ctx->fontImages[ctx->fontImageIdx + 1] != 0)
    {

        nvgImageSize(ctx, ctx->fontImages[ctx->fontImageIdx + 1], &iw, &ih);

    }

    else
    {

        nvgImageSize(ctx, ctx->fontImages[ctx->fontImageIdx], &iw, &ih);

        if (iw > ih)
            ih *= 2;
        else
            iw *= 2;

        if (iw > NVG_MAX_FONTIMAGE_SIZE || ih > NVG_MAX_FONTIMAGE_SIZE)
            iw = ih = NVG_MAX_FONTIMAGE_SIZE;

        ctx->fontImages[ctx->fontImageIdx + 1] = ctx->params.renderCreateTexture(ctx->params.userPtr, NVG_TEXTURE_ALPHA, iw, ih, 0, NULL);

    }

    ++ctx->fontImageIdx;

    fonsResetAtlas(ctx->fs, iw, ih);

    return 1;

}

static void renderText(struct nvg_context *ctx, struct nvg_vertex *verts, int nverts)
{

    struct nvg_state *state = getState(ctx);
    struct nvg_paint paint = state->fill;

    paint.image = ctx->fontImages[ctx->fontImageIdx];

    ctx->params.renderTriangles(ctx->params.userPtr, &paint, state->compositeoperation, &state->scissor, verts, nverts);
    ctx->drawCallCount++;
    ctx->textTriCount += nverts / 3;

}

float nvgText(struct nvg_context *ctx, float x, float y, const char *string, const char *end)
{

    struct nvg_state *state = getState(ctx);
    FONStextIter iter, prevIter;
    FONSquad q;
    struct nvg_vertex *verts;
    float scale = getFontScale(state) * ctx->devicePxRatio;
    float invscale = 1.0f / scale;
    int cverts = 0;
    int nverts = 0;

    if (end == NULL)
        end = string + strlen(string);

    if (state->fontId == FONS_INVALID)
        return x;

    fonsSetSize(ctx->fs, state->fontSize * scale);
    fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
    fonsSetBlur(ctx->fs, state->fontBlur * scale);
    fonsSetAlign(ctx->fs, state->textAlign);
    fonsSetFont(ctx->fs, state->fontId);

    cverts = maxi(2, (int)(end - string)) * 6;
    verts = allocTempVerts(ctx, cverts);

    if (verts == NULL)
        return x;

    fonsTextIterInit(ctx->fs, &iter, x * scale, y * scale, string, end, FONS_GLYPH_BITMAP_REQUIRED);

    prevIter = iter;

    while (fonsTextIterNext(ctx->fs, &iter, &q))
    {

        float c[4 * 2];

        if (iter.prevGlyphIndex == -1)
        {

            if (nverts != 0)
            {

                renderText(ctx, verts, nverts);

                nverts = 0;

            }

            if (!allocTextAtlas(ctx))
                break;

            iter = prevIter;

            fonsTextIterNext(ctx->fs, &iter, &q);

            if (iter.prevGlyphIndex == -1)
                break;

        }

        prevIter = iter;

        nvgTransformPoint(&c[0], &c[1], state->xform, q.x0 * invscale, q.y0 * invscale);
        nvgTransformPoint(&c[2], &c[3], state->xform, q.x1 * invscale, q.y0 * invscale);
        nvgTransformPoint(&c[4], &c[5], state->xform, q.x1 * invscale, q.y1 * invscale);
        nvgTransformPoint(&c[6], &c[7], state->xform, q.x0 * invscale, q.y1 * invscale);

        if (nverts + 6 <= cverts)
        {

            vset(&verts[nverts], c[0], c[1], q.s0, q.t0);

            nverts++;

            vset(&verts[nverts], c[4], c[5], q.s1, q.t1);

            nverts++;

            vset(&verts[nverts], c[2], c[3], q.s1, q.t0);

            nverts++;

            vset(&verts[nverts], c[0], c[1], q.s0, q.t0);

            nverts++;

            vset(&verts[nverts], c[6], c[7], q.s0, q.t1);

            nverts++;

            vset(&verts[nverts], c[4], c[5], q.s1, q.t1);

            nverts++;

        }

    }

    flushTextTexture(ctx);
    renderText(ctx, verts, nverts);

    return iter.nextx / scale;

}

int nvgTextGlyphPositions(struct nvg_context *ctx, float x, float y, const char *string, const char *end, struct nvg_glyphposition *positions, int maxPositions)
{

    struct nvg_state *state = getState(ctx);
    float scale = getFontScale(state) * ctx->devicePxRatio;
    float invscale = 1.0f / scale;
    FONStextIter iter, prevIter;
    FONSquad q;
    int npos = 0;

    if (state->fontId == FONS_INVALID)
        return 0;

    if (end == NULL)
        end = string + strlen(string);

    if (string == end)
        return 0;

    fonsSetSize(ctx->fs, state->fontSize * scale);
    fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
    fonsSetBlur(ctx->fs, state->fontBlur * scale);
    fonsSetAlign(ctx->fs, state->textAlign);
    fonsSetFont(ctx->fs, state->fontId);
    fonsTextIterInit(ctx->fs, &iter, x * scale, y * scale, string, end, FONS_GLYPH_BITMAP_OPTIONAL);

    prevIter = iter;

    while (fonsTextIterNext(ctx->fs, &iter, &q))
    {

        if (iter.prevGlyphIndex < 0 && allocTextAtlas(ctx))
        {

            iter = prevIter;

            fonsTextIterNext(ctx->fs, &iter, &q);

        }

        prevIter = iter;
        positions[npos].str = iter.str;
        positions[npos].x = iter.x * invscale;
        positions[npos].minx = minf(iter.x, q.x0) * invscale;
        positions[npos].maxx = maxf(iter.nextx, q.x1) * invscale;
        npos++;

        if (npos >= maxPositions)
            break;

    }

    return npos;

}

enum nvg_codepointType
{

    NVG_SPACE,
    NVG_NEWLINE,
    NVG_CHAR,
    NVG_CJK_CHAR,

};

int nvgTextBreakLines(struct nvg_context *ctx, const char *string, const char *end, float breakRowWidth, struct nvg_textrow *rows, int maxRows)
{

    struct nvg_state *state = getState(ctx);
    float scale = getFontScale(state) * ctx->devicePxRatio;
    float invscale = 1.0f / scale;
    FONStextIter iter, prevIter;
    FONSquad q;
    int nrows = 0;
    float rowStartX = 0;
    float rowWidth = 0;
    float rowMinX = 0;
    float rowMaxX = 0;
    const char *rowStart = NULL;
    const char *rowEnd = NULL;
    const char *wordStart = NULL;
    float wordStartX = 0;
    float wordMinX = 0;
    const char *breakEnd = NULL;
    float breakWidth = 0;
    float breakMaxX = 0;
    int type = NVG_SPACE, ptype = NVG_SPACE;
    unsigned int pcodepoint = 0;

    if (maxRows == 0)
        return 0;

    if (state->fontId == FONS_INVALID)
        return 0;

    if (end == NULL)
        end = string + strlen(string);

    if (string == end)
        return 0;

    fonsSetSize(ctx->fs, state->fontSize * scale);
    fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
    fonsSetBlur(ctx->fs, state->fontBlur * scale);
    fonsSetAlign(ctx->fs, state->textAlign);
    fonsSetFont(ctx->fs, state->fontId);

    breakRowWidth *= scale;

    fonsTextIterInit(ctx->fs, &iter, 0, 0, string, end, FONS_GLYPH_BITMAP_OPTIONAL);

    prevIter = iter;

    while (fonsTextIterNext(ctx->fs, &iter, &q))
    {

        if (iter.prevGlyphIndex < 0 && allocTextAtlas(ctx))
        {

            iter = prevIter;

            fonsTextIterNext(ctx->fs, &iter, &q);

        }

        prevIter = iter;

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
                if ((iter.codepoint >= 0x4E00 && iter.codepoint <= 0x9FFF) ||
                    (iter.codepoint >= 0x3000 && iter.codepoint <= 0x30FF) ||
                    (iter.codepoint >= 0xFF00 && iter.codepoint <= 0xFFEF) ||
                    (iter.codepoint >= 0x1100 && iter.codepoint <= 0x11FF) ||
                    (iter.codepoint >= 0x3130 && iter.codepoint <= 0x318F) ||
                    (iter.codepoint >= 0xAC00 && iter.codepoint <= 0xD7AF))
                    type = NVG_CJK_CHAR;
                else
                    type = NVG_CHAR;

                break;

        }

        if (type == NVG_NEWLINE)
        {

            rows[nrows].start = rowStart != NULL ? rowStart : iter.str;
            rows[nrows].end = rowEnd != NULL ? rowEnd : iter.str;
            rows[nrows].width = rowWidth * invscale;
            rows[nrows].minx = rowMinX * invscale;
            rows[nrows].maxx = rowMaxX * invscale;
            rows[nrows].next = iter.next;
            nrows++;

            if (nrows >= maxRows)
                return nrows;

            breakEnd = rowStart;
            breakWidth = 0.0;
            breakMaxX = 0.0;
            rowStart = NULL;
            rowEnd = NULL;
            rowWidth = 0;
            rowMinX = rowMaxX = 0;

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
                    wordStartX = iter.x;
                    wordMinX = q.x0 - rowStartX;
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
                    wordStartX = iter.x;
                    wordMinX = q.x0 - rowStartX;

                }

                if ((type == NVG_CHAR || type == NVG_CJK_CHAR) && nextWidth > breakRowWidth)
                {

                    if (breakEnd == rowStart)
                    {

                        rows[nrows].start = rowStart;
                        rows[nrows].end = iter.str;
                        rows[nrows].width = rowWidth * invscale;
                        rows[nrows].minx = rowMinX * invscale;
                        rows[nrows].maxx = rowMaxX * invscale;
                        rows[nrows].next = iter.str;
                        nrows++;

                        if (nrows >= maxRows)
                            return nrows;

                        rowStartX = iter.x;
                        rowStart = iter.str;
                        rowEnd = iter.next;
                        rowWidth = iter.nextx - rowStartX;
                        rowMinX = q.x0 - rowStartX;
                        rowMaxX = q.x1 - rowStartX;
                        wordStart = iter.str;
                        wordStartX = iter.x;
                        wordMinX = q.x0 - rowStartX;

                    }

                    else
                    {

                        rows[nrows].start = rowStart;
                        rows[nrows].end = breakEnd;
                        rows[nrows].width = breakWidth * invscale;
                        rows[nrows].minx = rowMinX * invscale;
                        rows[nrows].maxx = breakMaxX * invscale;
                        rows[nrows].next = wordStart;
                        nrows++;

                        if (nrows >= maxRows)
                            return nrows;

                        rowStartX = wordStartX;
                        rowStart = wordStart;
                        rowEnd = iter.next;
                        rowWidth = iter.nextx - rowStartX;
                        rowMinX = wordMinX;
                        rowMaxX = q.x1 - rowStartX;

                    }

                    breakEnd = rowStart;
                    breakWidth = 0.0;
                    breakMaxX = 0.0;

                }

            }

        }

        pcodepoint = iter.codepoint;
        ptype = type;

    }

    if (rowStart != NULL)
    {

        rows[nrows].start = rowStart;
        rows[nrows].end = rowEnd;
        rows[nrows].width = rowWidth * invscale;
        rows[nrows].minx = rowMinX * invscale;
        rows[nrows].maxx = rowMaxX * invscale;
        rows[nrows].next = end;
        nrows++;
    }

    return nrows;

}

void nvgTextMetrics(struct nvg_context *ctx, float *ascender, float *descender, float *lineh)
{

    struct nvg_state *state = getState(ctx);
    float scale = getFontScale(state) * ctx->devicePxRatio;
    float invscale = 1.0f / scale;

    if (state->fontId == FONS_INVALID)
        return;

    fonsSetSize(ctx->fs, state->fontSize * scale);
    fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
    fonsSetBlur(ctx->fs, state->fontBlur * scale);
    fonsSetAlign(ctx->fs, state->textAlign);
    fonsSetFont(ctx->fs, state->fontId);
    fonsVertMetrics(ctx->fs, ascender, descender, lineh);

    if (ascender != NULL)
        *ascender *= invscale;

    if (descender != NULL)
        *descender *= invscale;

    if (lineh != NULL)
        *lineh *= invscale;

}

