#if defined NANOVG_GL2_IMPLEMENTATION
#define NANOVG_GL2 1
#define NANOVG_GL_IMPLEMENTATION 1
#elif defined NANOVG_GL3_IMPLEMENTATION
#define NANOVG_GL3 1
#define NANOVG_GL_IMPLEMENTATION 1
#define NANOVG_GL_USE_UNIFORMBUFFER 1
#elif defined NANOVG_GLES2_IMPLEMENTATION
#define NANOVG_GLES2 1
#define NANOVG_GL_IMPLEMENTATION 1
#elif defined NANOVG_GLES3_IMPLEMENTATION
#define NANOVG_GLES3 1
#define NANOVG_GL_IMPLEMENTATION 1
#endif
#define NANOVG_GL_USE_STATE_FILTER (1)

enum nvg_gl_createflags
{

    NVG_ANTIALIAS = 1 << 0,
    NVG_STENCIL_STROKES = 1 << 1,
    NVG_DEBUG = 1 << 2,

};

enum nvg_gl_imageflags
{

    NVG_IMAGE_NODELETE = 1 << 16,

};

enum nvg_gl_uniformLoc
{

    GLNVG_LOC_VIEWSIZE,
    GLNVG_LOC_TEX,
    GLNVG_LOC_FRAG,
    GLNVG_MAX_LOCS

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

    GLNVG_FRAG_BINDING = 0,

};

struct nvg_gl_shader
{

    GLuint prog;
    GLuint frag;
    GLuint vert;
    GLint loc[GLNVG_MAX_LOCS];

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

enum nvg_gl_calltype
{

    GLNVG_NONE = 0,
    GLNVG_FILL,
    GLNVG_CONVEXFILL,
    GLNVG_STROKE,
    GLNVG_TRIANGLES,

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

struct nvg_gl_fraguniforms
{

#if NANOVG_GL_USE_UNIFORMBUFFER
    float scissorMat[12];
    float paintMat[12];
    struct nvg_color innerCol;
    struct nvg_color outerCol;
    float scissorExt[2];
    float scissorScale[2];
    float extent[2];
    float radius;
    float feather;
    float strokeMult;
    float strokeThr;
    int texType;
    int type;
#else
#define NANOVG_GL_UNIFORMARRAY_SIZE 11
    union
    {

        struct
        {

            float scissorMat[12];
            float paintMat[12];
            struct nvg_color innerCol;
            struct nvg_color outerCol;
            float scissorExt[2];
            float scissorScale[2];
            float extent[2];
            float radius;
            float feather;
            float strokeMult;
            float strokeThr;
            float texType;
            float type;

        };

        float uniformArray[NANOVG_GL_UNIFORMARRAY_SIZE][4];

    };
#endif

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
    struct nvg_gl_blend blendFunc;

};

struct nvg_context *nvg_create(int flags);
void nvg_delete(struct nvg_context *ctx);
