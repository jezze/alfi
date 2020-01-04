#define NVG_COMMANDSSIZE 256
#define NVG_POINTSSIZE 128
#define NVG_PATHSSIZE 16
#define NVG_VERTSSIZE 2048

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

struct nvg_scissor
{

    float xform[6];
    float extent[2];

};

struct nvg_point
{

    float x;
    float y;
    float dx;
    float dy;
    float len;
    float dmx;
    float dmy;

};

struct nvg_path
{

    int first;
    unsigned int count;
    unsigned char closed;
    struct nvg_vertex *fill;
    int nfill;
    int winding;
    int convex;

};

struct nvg_vertex
{

    float x;
    float y;
    float u;
    float v;

};

struct nvg_context
{

    struct nvg_point points[NVG_POINTSSIZE];
    unsigned int npoints;
    struct nvg_path paths[NVG_PATHSSIZE];
    unsigned int npaths;
    struct nvg_vertex verts[NVG_VERTSSIZE];
    unsigned int nverts;
    float commands[NVG_COMMANDSSIZE];
    unsigned int ncommands;
    float bounds[4];
    float xform[6];

};

struct nvg_color nvg_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
struct nvg_color nvg_rgbaf(float r, float g, float b, float a);
struct nvg_color nvg_premulrgba(struct nvg_color c);
void nvg_paint_color(struct nvg_paint *paint, float r, float g, float b, float a);
void nvg_paint_image(struct nvg_paint *paint, float ox, float oy, float ex, float ey, float angle, int image, float alpha);
void nvg_paint_boxgradient(struct nvg_paint *paint, float x, float y, float w, float h, float r, float f, struct nvg_color icol, struct nvg_color ocol);
void nvg_paint_lineargradient(struct nvg_paint *paint, float sx, float sy, float ex, float ey, struct nvg_color icol, struct nvg_color ocol);
void nvg_paint_radialgradient(struct nvg_paint *paint, float cx, float cy, float inr, float outr, struct nvg_color icol, struct nvg_color ocol);
void nvg_setvertex(struct nvg_vertex *vtx, float x, float y, float u, float v);
void nvg_xform_identity(float *t);
void nvg_xform_translate(float *t, float tx, float ty);
void nvg_xform_scale(float *t, float sx, float sy);
void nvg_xform_rotate(float *t, float a);
void nvg_xform_skewx(float *t, float a);
void nvg_xform_skewy(float *t, float a);
void nvg_xform_multiply(float *t, const float *s);
void nvg_xform_premultiply(float *t, const float *s);
void nvg_xform_inverse(float *t, const float *s);
void nvg_getpoints(float *dstx, float *dsty, const float *xform, float srcx, float srcy);
void nvg_transform(struct nvg_context *ctx, float a, float b, float c, float d, float e, float f);
void nvg_translate(struct nvg_context *ctx, float x, float y);
void nvg_rotate(struct nvg_context *ctx, float angle);
void nvg_skewx(struct nvg_context *ctx, float angle);
void nvg_skewy(struct nvg_context *ctx, float angle);
void nvg_scale(struct nvg_context *ctx, float x, float y);
void nvg_scissor_init(struct nvg_scissor *scissor);
void nvg_scissor_set(struct nvg_scissor *scissor, float *xform, float x, float y, float w, float h);
void nvg_path_moveto(struct nvg_context *ctx, float x, float y);
void nvg_path_lineto(struct nvg_context *ctx, float x, float y);
void nvg_path_bezierto(struct nvg_context *ctx, float c1x, float c1y, float c2x, float c2y, float x, float y);
void nvg_path_begin(struct nvg_context *ctx);
void nvg_path_close(struct nvg_context *ctx);
void nvg_path_solid(struct nvg_context *ctx);
void nvg_path_hole(struct nvg_context *ctx);
void nvg_path_rect(struct nvg_context *ctx, float x, float y, float w, float h);
void nvg_path_roundedrect(struct nvg_context *ctx, float x, float y, float w, float h, float r);
void nvg_path_roundedrectvarying(struct nvg_context *ctx, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft);
void nvg_path_ellipse(struct nvg_context *ctx, float cx, float cy, float rx, float ry);
void nvg_path_circle(struct nvg_context *ctx, float cx, float cy, float r);
void nvg_flatten(struct nvg_context *ctx);
void nvg_expand(struct nvg_context *ctx);
void nvg_reset(struct nvg_context *ctx);
