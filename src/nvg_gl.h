enum nvg_gl_texturetype
{

    NVG_TEXTURE_ALPHA = 0x01,
    NVG_TEXTURE_RGBA = 0x02

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

struct nvg_gl_shader
{

    GLuint prog;
    GLuint frag;
    GLuint vert;
    GLint loc[3];

};

struct nvg_gl_texture
{

    int id;
    GLuint tex;
    int width;
    int height;
    int type;
    int flags;

};

struct nvg_gl_blend
{

    GLenum srcrgb;
    GLenum dstrgb;
    GLenum srcalpha;
    GLenum dstalpha;

};

struct nvg_gl_call
{

    int type;
    int image;
    int pathoffset;
    int pathcount;
    int triangleoffset;
    int trianglecount;
    int uniformoffset;
    struct nvg_gl_blend blendfunc;

};

struct nvg_gl_path
{

    int filloffset;
    int fillcount;

};

struct nvg_gl_context
{

    float view[2];
    struct nvg_gl_shader shader;
    struct nvg_gl_texture *textures;
    unsigned int ntextures;
    unsigned int ctextures;
    unsigned int textureid;
    GLuint vertBuf;
    GLuint vertArr;
    GLuint fragBuf;
    unsigned int fragsize;
    struct nvg_gl_call *calls;
    unsigned int ccalls;
    unsigned int ncalls;
    struct nvg_gl_path *paths;
    unsigned int cpaths;
    unsigned int npaths;
    struct nvg_vertex *verts;
    unsigned int cverts;
    unsigned int nverts;
    unsigned char *uniforms;
    unsigned int cuniforms;
    unsigned int nuniforms;
    struct nvg_gl_blend blendfunc;
    int fontimage;

};

struct nvg_gl_texture *nvg_gl_findtexture(struct nvg_gl_context *glctx, int id);
void nvg_gl_flush(struct nvg_gl_context *glctx);
void nvg_gl_reset(struct nvg_gl_context *glctx, float width, float height);
void nvg_gl_fill(struct nvg_gl_context *glctx, struct nvg_context *ctx, struct nvg_paint *paint, struct nvg_scissor *scissor);
int nvg_gl_texture_update(struct nvg_gl_context *glctx, int image, int x, int y, int w, int h, const unsigned char *data);
int nvg_gl_texture_create(struct nvg_gl_context *glctx, int type, int w, int h, int imageFlags, const unsigned char *data);
void nvg_gl_texture_destroy(struct nvg_gl_context *glctx, int id);
void nvg_gl_create(struct nvg_gl_context *glctx, int w, int h);
void nvg_gl_render_paths(struct nvg_gl_context *glctx, struct nvg_paint *paint, struct nvg_scissor *scissor, const float *bounds, const struct nvg_path *paths, int npaths);
void nvg_gl_render_vertices(struct nvg_gl_context *glctx, struct nvg_paint *paint, struct nvg_scissor *scissor, const struct nvg_vertex *verts, int nverts);
void nvg_gl_destroy(struct nvg_gl_context *glctx);
