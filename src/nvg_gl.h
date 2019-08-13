#define NVG_GL_MAXFONTIMAGES 4

enum nvg_gl_createflags
{

    NVG_ANTIALIAS = 1 << 0,
    NVG_STENCIL_STROKES = 1 << 1

};

enum nvg_gl_imageflags
{

    NVG_IMAGE_NODELETE = 1 << 16,

};

enum nvg_gl_uniformLoc
{

    NVG_GL_LOC_VIEWSIZE,
    NVG_GL_LOC_TEX,
    NVG_GL_LOC_FRAG,
    NVG_GL_MAX_LOCS

};

enum nvg_gl_shadertype
{

    NSVG_SHADER_FILLGRAD,
    NSVG_SHADER_FILLIMG,
    NSVG_SHADER_SIMPLE,
    NSVG_SHADER_IMG

};

enum nvg_gl_uniformbindings
{

    NVG_GL_FRAG_BINDING = 0,

};

enum nvg_gl_texturetype
{

    NVG_TEXTURE_ALPHA = 0x01,
    NVG_TEXTURE_RGBA = 0x02

};

enum nvg_gl_calltype
{

    NVG_GL_NONE = 0,
    NVG_GL_FILL,
    NVG_GL_CONVEXFILL,
    NVG_GL_STROKE,
    NVG_GL_TRIANGLES,

};

struct nvg_gl_shader
{

    GLuint prog;
    GLuint frag;
    GLuint vert;
    GLint loc[NVG_GL_MAX_LOCS];

};

struct nvg_gl_texture
{

    int id;
    GLuint tex;
    int width, height;
    int type;
    int flags;

};

struct nvg_gl_blend
{

    GLenum srcRGB;
    GLenum dstRGB;
    GLenum srcAlpha;
    GLenum dstAlpha;

};

struct nvg_gl_call
{

    int type;
    int image;
    int pathOffset;
    int pathCount;
    int triangleOffset;
    int triangleCount;
    int uniformOffset;
    struct nvg_gl_blend blendFunc;

};

struct nvg_gl_path
{

    int fillOffset;
    int fillCount;
    int strokeOffset;
    int strokeCount;

};

struct nvg_gl_context
{

    struct nvg_gl_shader shader;
    struct nvg_gl_texture *textures;
    float view[2];
    int ntextures;
    int ctextures;
    int textureId;
    GLuint vertBuf;
    GLuint vertArr;
    GLuint fragBuf;
    int fragSize;
    int flags;
    struct nvg_gl_call *calls;
    int ccalls;
    int ncalls;
    struct nvg_gl_path *paths;
    int cpaths;
    int npaths;
    struct nvg_vertex *verts;
    int cverts;
    int nverts;
    unsigned char *uniforms;
    int cuniforms;
    int nuniforms;
    GLuint boundTexture;
    GLuint stencilMask;
    GLenum stencilFunc;
    GLint stencilFuncRef;
    GLuint stencilFuncMask;
    int edgeAntiAlias;
    struct nvg_gl_blend blendFunc;
    int fontImages[NVG_GL_MAXFONTIMAGES];
    int fontImageIdx;

};

void nvg_gl_flush(struct nvg_gl_context *gl);
void nvg_gl_beginframe(struct nvg_gl_context *glctx, struct nvg_context *ctx, float windowWidth, float windowHeight);
void nvg_gl_cancelframe(struct nvg_gl_context *glctx, struct nvg_context *ctx);
void nvg_gl_endframe(struct nvg_gl_context *glctx, struct nvg_context *ctx);
int nvg_gl_createimagefile(struct nvg_gl_context *glctx, struct nvg_context *ctx, const char *filename, int imageFlags);
int nvg_gl_createimagemem(struct nvg_gl_context *glctx, struct nvg_context *ctx, int imageFlags, unsigned char *data, int ndata);
int nvg_gl_createimagergba(struct nvg_gl_context *glctx, struct nvg_context *ctx, int w, int h, int imageFlags, const unsigned char *data);
void nvg_gl_updateimage(struct nvg_gl_context *glctx, int image, const unsigned char *data);
void nvg_gl_imagesize(struct nvg_gl_context *glctx, int image, int *w, int *h);
void nvg_gl_deletetexture(struct nvg_gl_context *glctx, int id);
void nvg_gl_fill(struct nvg_gl_context *glctx, struct nvg_context *ctx, struct nvg_state *state);
void nvg_gl_stroke(struct nvg_gl_context *glctx, struct nvg_context *ctx, struct nvg_state *state);
float nvg_gl_text(struct nvg_gl_context *glctx, struct nvg_context *ctx, struct nvg_state *state, struct fons_context *fsctx, float x, float y, const char *string, const char *end);
void nvg_gl_create(struct nvg_gl_context *glctx, int w, int h, int flags);
void nvg_gl_delete(struct nvg_gl_context *glctx);
void nvg_gl_compop(struct nvg_context *ctx, int op);
