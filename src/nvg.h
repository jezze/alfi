#define NVG_COMMANDSSIZE 256
#define NVG_POINTSSIZE 128
#define NVG_PATHSSIZE 16
#define NVG_VERTSSIZE 2048

enum nvg_codepointtype
{

    NVG_SPACE,
    NVG_NEWLINE,
    NVG_CHAR,
    NVG_CJK_CHAR,

};

enum nvg_winding
{

    NVG_CCW = 1,
    NVG_CW = 2

};

enum nvg_solidity
{

    NVG_SOLID = 1,
    NVG_HOLE = 2

};

enum nvg_linecap
{

    NVG_BUTT,
    NVG_ROUND,
    NVG_SQUARE,
    NVG_BEVEL,
    NVG_MITER

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
    NVG_XOR

};

enum nvg_imageflags
{

    NVG_IMAGE_GENERATE_MIPMAPS = 1 << 0,
    NVG_IMAGE_REPEATX = 1 << 1,
    NVG_IMAGE_REPEATY = 1 << 2,
    NVG_IMAGE_FLIPY = 1 << 3,
    NVG_IMAGE_PREMULTIPLIED = 1 << 4,
    NVG_IMAGE_NEAREST = 1 << 5

};

enum nvg_commands
{

    NVG_MOVETO = 0,
    NVG_LINETO = 1,
    NVG_BEZIERTO = 2,
    NVG_CLOSE = 3,
    NVG_WINDING = 4

};

enum nvg_pointflags
{

    NVG_PT_CORNER = 0x01,
    NVG_PT_LEFT = 0x02,
    NVG_PT_BEVEL = 0x04,
    NVG_PR_INNERBEVEL = 0x08

};

struct nvg_color
{

    float r;
    float g;
    float b;
    float a;

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

struct nvg_scissor
{

    float xform[6];
    float extent[2];

};

struct nvg_vertex
{

    float x;
    float y;
    float u;
    float v;

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

    struct nvg_point points[NVG_POINTSSIZE];
    int npoints;
    struct nvg_path paths[NVG_PATHSSIZE];
    int npaths;
    struct nvg_vertex verts[NVG_VERTSSIZE];
    int nverts;
    float bounds[4];

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

};

struct nvg_context
{

    float commands[NVG_COMMANDSSIZE];
    int ncommands;
    float commandx, commandy;
    struct nvg_state state;
    struct nvg_pathcache cache;
    float tessTol;
    float distTol;
    float fringeWidth;
    int drawCallCount;
    int fillTriCount;
    int strokeTriCount;
    int textTriCount;

};

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
void nvg_setstrokecolor(struct nvg_context *ctx, struct nvg_color color);
void nvg_setstrokepaint(struct nvg_context *ctx, struct nvg_paint paint);
void nvg_setfillcolor(struct nvg_context *ctx, struct nvg_color color);
void nvg_setfillpaint(struct nvg_context *ctx, struct nvg_paint paint);
void nvgTransform(struct nvg_context *ctx, float a, float b, float c, float d, float e, float f);
void nvgTranslate(struct nvg_context *ctx, float x, float y);
void nvgRotate(struct nvg_context *ctx, float angle);
void nvgSkewX(struct nvg_context *ctx, float angle);
void nvgSkewY(struct nvg_context *ctx, float angle);
void nvgScale(struct nvg_context *ctx, float x, float y);
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
void nvgLinearGradient(struct nvg_paint *paint, float sx, float sy, float ex, float ey, struct nvg_color icol, struct nvg_color ocol);
void nvgBoxGradient(struct nvg_paint *paint, float x, float y, float w, float h, float r, float f, struct nvg_color icol, struct nvg_color ocol);
void nvgRadialGradient(struct nvg_paint *paint, float cx, float cy, float inr, float outr, struct nvg_color icol, struct nvg_color ocol);
void nvgImagePattern(struct nvg_paint *paint, float ox, float oy, float ex, float ey, float angle, int image, float alpha);
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
int nvg_expand_fill(struct nvg_context *ctx, float w, int linejoin, float miterLimit);
int nvg_expand_stroke(struct nvg_context *ctx, float w, float fringe, int linecap, int linejoin, float miterLimit);
void nvg_flatten_paths(struct nvg_context *ctx);
struct nvg_vertex *nvg_allocverts(struct nvg_context *ctx, int nverts);
void nvg_init(struct nvg_context *ctx);
