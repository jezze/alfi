#define NVG_PI 3.14159265358979323846264338327f
#define NVG_MAX_FONTIMAGES 4
#define NVG_MAX_STATES 32

struct nvg_color
{

    union
    {

        float rgba[4];

        struct
        {

            float r, g, b, a;

        };

    };

};

struct nvg_paint
{

    float xform[6];
    float extent[2];
    float radius;
    float feather;
    struct nvg_color innerColor;
    struct nvg_color outerColor;
    int image;

};

enum nvg_winding
{

    NVG_CCW = 1,
    NVG_CW = 2,

};

enum nvg_solidity
{

    NVG_SOLID = 1,
    NVG_HOLE = 2,

};

enum nvg_linecap
{

    NVG_BUTT,
    NVG_ROUND,
    NVG_SQUARE,
    NVG_BEVEL,
    NVG_MITER,

};

enum nvg_align
{

    NVG_ALIGN_LEFT = 1 << 0,
    NVG_ALIGN_CENTER = 1 << 1,
    NVG_ALIGN_RIGHT = 1 << 2,
    NVG_ALIGN_TOP = 1 << 3,
    NVG_ALIGN_MIDDLE = 1 << 4,
    NVG_ALIGN_BOTTOM = 1 << 5,
    NVG_ALIGN_BASELINE = 1 << 6,

};

enum nvg_blendfactor
{

    NVG_ZERO = 1 << 0,
    NVG_ONE = 1 << 1,
    NVG_SRC_COLOR = 1 << 2,
    NVG_ONE_MINUS_SRC_COLOR = 1 << 3,
    NVG_DST_COLOR = 1 << 4,
    NVG_ONE_MINUS_DST_COLOR = 1 << 5,
    NVG_SRC_ALPHA = 1 << 6,
    NVG_ONE_MINUS_SRC_ALPHA = 1 << 7,
    NVG_DST_ALPHA = 1 << 8,
    NVG_ONE_MINUS_DST_ALPHA = 1 << 9,
    NVG_SRC_ALPHA_SATURATE = 1 << 10,

};

enum nvg_compositeoperation
{

    NVG_SOURCE_OVER,
    NVG_SOURCE_IN,
    NVG_SOURCE_OUT,
    NVG_ATOP,
    NVG_DESTINATION_OVER,
    NVG_DESTINATION_IN,
    NVG_DESTINATION_OUT,
    NVG_DESTINATION_ATOP,
    NVG_LIGHTER,
    NVG_COPY,
    NVG_XOR,

};

struct nvg_compositeoperationstate
{

    int srcRGB;
    int dstRGB;
    int srcAlpha;
    int dstAlpha;

};

struct nvg_glyphposition
{

    const char *str;
    float x;
    float minx, maxx;

};

struct nvg_textrow
{

    const char *start;
    const char *end;
    const char *next;
    float width;
    float minx, maxx;

};

enum nvg_imageflags
{

    NVG_IMAGE_GENERATE_MIPMAPS = 1 << 0,
    NVG_IMAGE_REPEATX = 1 << 1,
    NVG_IMAGE_REPEATY = 1 << 2,
    NVG_IMAGE_FLIPY = 1 << 3,
    NVG_IMAGE_PREMULTIPLIED = 1 << 4,
    NVG_IMAGE_NEAREST = 1 << 5,

};

enum nvg_texture
{

    NVG_TEXTURE_ALPHA = 0x01,
    NVG_TEXTURE_RGBA = 0x02,

};

struct nvg_scissor
{

    float xform[6];
    float extent[2];

};

struct nvg_vertex
{

    float x, y, u, v;

};

struct nvg_path
{

    int first;
    int count;
    unsigned char closed;
    int nbevel;
    struct nvg_vertex *fill;
    int nfill;
    struct nvg_vertex *stroke;
    int nstroke;
    int winding;
    int convex;

};

enum nvg_commands
{

    NVG_MOVETO = 0,
    NVG_LINETO = 1,
    NVG_BEZIERTO = 2,
    NVG_CLOSE = 3,
    NVG_WINDING = 4,

};

enum nvg_pointflags
{

    NVG_PT_CORNER = 0x01,
    NVG_PT_LEFT = 0x02,
    NVG_PT_BEVEL = 0x04,
    NVG_PR_INNERBEVEL = 0x08,

};

struct nvg_point
{

    float x,y;
    float dx, dy;
    float len;
    float dmx, dmy;
    unsigned char flags;

};

struct nvg_pathcache
{

    struct nvg_point *points;
    int npoints;
    int cpoints;
    struct nvg_path *paths;
    int npaths;
    int cpaths;
    struct nvg_vertex *verts;
    int nverts;
    int cverts;
    float bounds[4];

};

struct nvg_params
{

    void *userPtr;
    int edgeAntiAlias;
    int (*renderCreate)(void *uptr);
    int (*renderCreateTexture)(void *uptr, int type, int w, int h, int imageFlags, const unsigned char *data);
    int (*renderDeleteTexture)(void *uptr, int image);
    int (*renderUpdateTexture)(void *uptr, int image, int x, int y, int w, int h, const unsigned char *data);
    int (*renderGetTextureSize)(void *uptr, int image, int *w, int *h);
    void (*renderViewport)(void *uptr, float width, float height, float devicePixelRatio);
    void (*renderCancel)(void *uptr);
    void (*renderFlush)(void *uptr);
    void (*renderFill)(void *uptr, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, float fringe, const float *bounds, const struct nvg_path *paths, int npaths);
    void (*renderStroke)(void *uptr, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, float fringe, float strokeWidth, const struct nvg_path *paths, int npaths);
    void (*renderTriangles)(void *uptr, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, const struct nvg_vertex *verts, int nverts);
    void (*renderDelete)(void *uptr);

};

struct nvg_state
{

    struct nvg_compositeoperationstate compositeoperation;
    int shapeAntiAlias;
    struct nvg_paint fill;
    struct nvg_paint stroke;
    float strokeWidth;
    float miterLimit;
    int linejoin;
    int linecap;
    float xform[6];
    struct nvg_scissor scissor;
    float fontSize;
    float letterSpacing;
    float fontBlur;
    int textAlign;
    int fontId;

};

struct nvg_context
{

    struct nvg_params params;
    float *commands;
    int ccommands;
    int ncommands;
    float commandx, commandy;
    struct nvg_state states[NVG_MAX_STATES];
    int nstates;
    struct nvg_pathcache *cache;
    float tessTol;
    float distTol;
    float fringeWidth;
    float devicePxRatio;
    struct FONScontext *fs;
    int fontImages[NVG_MAX_FONTIMAGES];
    int fontImageIdx;
    int drawCallCount;
    int fillTriCount;
    int strokeTriCount;
    int textTriCount;

};

void nvgBeginFrame(struct nvg_context *ctx, float windowWidth, float windowHeight, float devicePixelRatio);
void nvgCancelFrame(struct nvg_context *ctx);
void nvgEndFrame(struct nvg_context *ctx);
void nvgGlobalCompositeOperation(struct nvg_context *ctx, int op);
void nvgGlobalCompositeBlendFunc(struct nvg_context *ctx, int sfactor, int dfactor);
void nvgGlobalCompositeBlendFuncSeparate(struct nvg_context *ctx, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha);
struct nvg_color nvgRGB(unsigned char r, unsigned char g, unsigned char b);
struct nvg_color nvgRGBf(float r, float g, float b);
struct nvg_color nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
struct nvg_color nvgRGBAf(float r, float g, float b, float a);
struct nvg_color nvgLerpRGBA(struct nvg_color c0, struct nvg_color c1, float u);
struct nvg_color nvgTransRGBA(struct nvg_color c0, unsigned char a);
struct nvg_color nvgTransRGBAf(struct nvg_color c0, float a);
struct nvg_color nvgHSL(float h, float s, float l);
struct nvg_color nvgHSLA(float h, float s, float l, unsigned char a);
void nvgSave(struct nvg_context *ctx);
void nvgRestore(struct nvg_context *ctx);
void nvgReset(struct nvg_context *ctx);
void nvgShapeAntiAlias(struct nvg_context *ctx, int enabled);
void nvgStrokeColor(struct nvg_context *ctx, struct nvg_color color);
void nvgStrokePaint(struct nvg_context *ctx, struct nvg_paint paint);
void nvgFillColor(struct nvg_context *ctx, struct nvg_color color);
void nvgFillPaint(struct nvg_context *ctx, struct nvg_paint paint);
void nvgMiterLimit(struct nvg_context *ctx, float limit);
void nvgStrokeWidth(struct nvg_context *ctx, float size);
void nvgLineCap(struct nvg_context *ctx, int cap);
void nvgLineJoin(struct nvg_context *ctx, int join);
void nvgResetTransform(struct nvg_context *ctx);
void nvgTransform(struct nvg_context *ctx, float a, float b, float c, float d, float e, float f);
void nvgTranslate(struct nvg_context *ctx, float x, float y);
void nvgRotate(struct nvg_context *ctx, float angle);
void nvgSkewX(struct nvg_context *ctx, float angle);
void nvgSkewY(struct nvg_context *ctx, float angle);
void nvgScale(struct nvg_context *ctx, float x, float y);
void nvgCurrentTransform(struct nvg_context *ctx, float *xform);
void nvgTransformIdentity(float *dst);
void nvgTransformTranslate(float *dst, float tx, float ty);
void nvgTransformScale(float *dst, float sx, float sy);
void nvgTransformRotate(float *dst, float a);
void nvgTransformSkewX(float *dst, float a);
void nvgTransformSkewY(float *dst, float a);
void nvgTransformMultiply(float *dst, const float *src);
void nvgTransformPremultiply(float *dst, const float *src);
int nvgTransformInverse(float *dst, const float *src);
void nvgTransformPoint(float *dstx, float *dsty, const float *xform, float srcx, float srcy);
float nvgDegToRad(float deg);
float nvgRadToDeg(float rad);
int nvgCreateImage(struct nvg_context *ctx, const char *filename, int imageFlags);
int nvgCreateImageMem(struct nvg_context *ctx, int imageFlags, unsigned char *data, int ndata);
int nvgCreateImageRGBA(struct nvg_context *ctx, int w, int h, int imageFlags, const unsigned char *data);
void nvgUpdateImage(struct nvg_context *ctx, int image, const unsigned char *data);
void nvgImageSize(struct nvg_context *ctx, int image, int *w, int *h);
void nvgDeleteImage(struct nvg_context *ctx, int image);
struct nvg_paint nvgLinearGradient(struct nvg_context *ctx, float sx, float sy, float ex, float ey, struct nvg_color icol, struct nvg_color ocol);
struct nvg_paint nvgBoxGradient(struct nvg_context *ctx, float x, float y, float w, float h, float r, float f, struct nvg_color icol, struct nvg_color ocol);
struct nvg_paint nvgRadialGradient(struct nvg_context *ctx, float cx, float cy, float inr, float outr, struct nvg_color icol, struct nvg_color ocol);
struct nvg_paint nvgImagePattern(struct nvg_context *ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha);
void nvgScissor(struct nvg_context *ctx, float x, float y, float w, float h);
void nvgIntersectScissor(struct nvg_context *ctx, float x, float y, float w, float h);
void nvgResetScissor(struct nvg_context *ctx);
void nvgBeginPath(struct nvg_context *ctx);
void nvgMoveTo(struct nvg_context *ctx, float x, float y);
void nvgLineTo(struct nvg_context *ctx, float x, float y);
void nvgBezierTo(struct nvg_context *ctx, float c1x, float c1y, float c2x, float c2y, float x, float y);
void nvgQuadTo(struct nvg_context *ctx, float cx, float cy, float x, float y);
void nvgArcTo(struct nvg_context *ctx, float x1, float y1, float x2, float y2, float radius);
void nvgClosePath(struct nvg_context *ctx);
void nvgPathWinding(struct nvg_context *ctx, int dir);
void nvgArc(struct nvg_context *ctx, float cx, float cy, float r, float a0, float a1, int dir);
void nvgRect(struct nvg_context *ctx, float x, float y, float w, float h);
void nvgRoundedRect(struct nvg_context *ctx, float x, float y, float w, float h, float r);
void nvgRoundedRectVarying(struct nvg_context *ctx, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft);
void nvgEllipse(struct nvg_context *ctx, float cx, float cy, float rx, float ry);
void nvgCircle(struct nvg_context *ctx, float cx, float cy, float r);
void nvgFill(struct nvg_context *ctx);
void nvgStroke(struct nvg_context *ctx);
int nvgCreateFont(struct nvg_context *ctx, const char *name, const char *filename);
int nvgCreateFontMem(struct nvg_context *ctx, const char *name, unsigned char *data, int ndata, int freeData);
void nvgFontSize(struct nvg_context *ctx, float size);
void nvgFontBlur(struct nvg_context *ctx, float blur);
void nvgTextLetterSpacing(struct nvg_context *ctx, float spacing);
void nvgTextAlign(struct nvg_context *ctx, int align);
void nvgFontFaceId(struct nvg_context *ctx, int font);
float nvgText(struct nvg_context *ctx, float x, float y, const char *string, const char *end);
int nvgTextGlyphPositions(struct nvg_context *ctx, float x, float y, const char *string, const char *end, struct nvg_glyphposition *positions, int maxPositions);
void nvgTextMetrics(struct nvg_context *ctx, float *ascender, float *descender, float *lineh);
int nvgTextBreakLines(struct nvg_context *ctx, const char *string, const char *end, float breakRowWidth, struct nvg_textrow *rows, int maxRows);
struct nvg_context *nvgCreateInternal(struct nvg_params *params);
void nvgDeleteInternal(struct nvg_context *ctx);
