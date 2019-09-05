#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_truetype.h"
#include "fons.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "nvg.h"
#include "nvg_gl.h"

#define NVG_GL_VERSION_GL2 1
#define NVG_GL_VERSION_GL3 0
#define NVG_GL_VERSION_GLES2 0
#define NVG_GL_VERSION_GLES3 0
#define NVG_GL_USEUNIFORMBUFFER 0
#define NVG_GL_USESTATEFILTER 1
#define NVG_GL_UNIFORMARRAYSIZE 11

struct nvg_gl_fraguniforms
{

#if NVG_GL_USEUNIFORMBUFFER
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

        float uniformArray[NVG_GL_UNIFORMARRAYSIZE][4];

    };
#endif

};

static int maxi(int a, int b)
{

    return a > b ? a : b;

}

static float clampf(float a, float mn, float mx)
{

    return a < mn ? mn : (a > mx ? mx : a);

}

#ifdef NVG_GL_VERSION_GLES2
static unsigned int nearestPow2(unsigned int num)
{

    unsigned n = num > 0 ? num - 1 : 0;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;

}
#endif

static void bindTexture(struct nvg_gl_context *glctx, GLuint tex)
{

#if NVG_GL_USESTATEFILTER
    if (glctx->boundTexture != tex)
    {

        glctx->boundTexture = tex;

        glBindTexture(GL_TEXTURE_2D, tex);

    }
#else
    glBindTexture(GL_TEXTURE_2D, tex);
#endif

}

static void stencilMask(struct nvg_gl_context *glctx, GLuint mask)
{

#if NVG_GL_USESTATEFILTER
    if (glctx->stencilMask != mask)
    {

        glctx->stencilMask = mask;

        glStencilMask(mask);

    }
#else
    glStencilMask(mask);
#endif

}

static void stencilFunc(struct nvg_gl_context *glctx, GLenum func, GLint ref, GLuint mask)
{

#if NVG_GL_USESTATEFILTER
    if ((glctx->stencilFunc != func) || (glctx->stencilFuncRef != ref) || (glctx->stencilFuncMask != mask))
    {

        glctx->stencilFunc = func;
        glctx->stencilFuncRef = ref;
        glctx->stencilFuncMask = mask;

        glStencilFunc(func, ref, mask);

    }
#else
    glStencilFunc(func, ref, mask);
#endif

}

static void blendFuncSeparate(struct nvg_gl_context *glctx, const struct nvg_gl_blend *blend)
{

#if NVG_GL_USESTATEFILTER
    if ((glctx->blendFunc.srcRGB != blend->srcRGB) || (glctx->blendFunc.dstRGB != blend->dstRGB) || (glctx->blendFunc.srcAlpha != blend->srcAlpha) || (glctx->blendFunc.dstAlpha != blend->dstAlpha))
    {

        glctx->blendFunc = *blend;

        glBlendFuncSeparate(blend->srcRGB, blend->dstRGB, blend->srcAlpha, blend->dstAlpha);

    }
#else
    glBlendFuncSeparate(blend->srcRGB, blend->dstRGB, blend->srcAlpha, blend->dstAlpha);
#endif

}

static struct nvg_gl_texture *findTexture(struct nvg_gl_context *glctx, int id)
{

    unsigned int i;

    for (i = 0; i < glctx->ntextures; i++)
    {

        if (glctx->textures[i].id == id)
            return &glctx->textures[i];

    }

    return 0;

}

static struct nvg_gl_texture *allocTexture(struct nvg_gl_context *glctx)
{

    struct nvg_gl_texture *tex = findTexture(glctx, 0);

    if (!tex)
    {

        if (glctx->ntextures + 1 > glctx->ctextures)
        {

            int ctextures = maxi(glctx->ntextures + 1, 4) +  glctx->ctextures / 2;
            struct nvg_gl_texture *textures = realloc(glctx->textures, sizeof (struct nvg_gl_texture) * ctextures);

            if (!textures)
                return 0;

            glctx->textures = textures;
            glctx->ctextures = ctextures;

        }

        tex = &glctx->textures[glctx->ntextures++];

    }

    memset(tex, 0, sizeof (struct nvg_gl_texture));

    tex->id = ++glctx->textureId;

    return tex;

}

static void dumpShaderError(GLuint shader, const char *name, const char *type)
{

    GLchar str[512 + 1];
    GLsizei len = 0;

    glGetShaderInfoLog(shader, 512, &len, str);

    if (len > 512)
        len = 512;

    str[len] = '\0';

    printf("Shader %s/%s error:\n%s\n", name, type, str);

}

static void dumpProgramError(GLuint prog, const char *name)
{

    GLchar str[512 + 1];
    GLsizei len = 0;

    glGetProgramInfoLog(prog, 512, &len, str);

    if (len > 512)
        len = 512;

    str[len] = '\0';

    printf("Program %s error:\n%s\n", name, str);

}

static void checkError(struct nvg_gl_context *glctx, const char *str)
{

    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
    {

        printf("Error %08x after %s\n", err, str);
        
        return;

    }

}

static int createShader(struct nvg_gl_shader *shader, const char *name, const char *header, const char *opts, const char *vshader, const char *fshader)
{

    GLint status;
    GLuint prog, vert, frag;
    const char *str[3];

    str[0] = header;
    str[1] = opts ? opts : "";

    memset(shader, 0, sizeof (struct nvg_gl_shader));

    prog = glCreateProgram();
    vert = glCreateShader(GL_VERTEX_SHADER);
    frag = glCreateShader(GL_FRAGMENT_SHADER);
    str[2] = vshader;

    glShaderSource(vert, 3, str, 0);

    str[2] = fshader;

    glShaderSource(frag, 3, str, 0);
    glCompileShader(vert);
    glGetShaderiv(vert, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE)
    {

        dumpShaderError(vert, name, "vert");

        return 0;

    }

    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE)
    {

        dumpShaderError(frag, name, "frag");

        return 0;

    }

    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glBindAttribLocation(prog, 0, "vertex");
    glBindAttribLocation(prog, 1, "tcoord");
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {

        dumpProgramError(prog, name);

        return 0;

    }

    shader->prog = prog;
    shader->vert = vert;
    shader->frag = frag;

    return 1;

}

static void deleteShader(struct nvg_gl_shader *shader)
{

    if (shader->prog)
        glDeleteProgram(shader->prog);

    if (shader->vert)
        glDeleteShader(shader->vert);

    if (shader->frag)
        glDeleteShader(shader->frag);

}

static void getUniforms(struct nvg_gl_shader *shader)
{

    shader->loc[NVG_GL_LOC_VIEWSIZE] = glGetUniformLocation(shader->prog, "viewSize");
    shader->loc[NVG_GL_LOC_TEX] = glGetUniformLocation(shader->prog, "tex");
#if NVG_GL_USEUNIFORMBUFFER
    shader->loc[NVG_GL_LOC_FRAG] = glGetUniformBlockIndex(shader->prog, "frag");
#else
    shader->loc[NVG_GL_LOC_FRAG] = glGetUniformLocation(shader->prog, "frag");
#endif

}

static int renderCreate(struct nvg_gl_context *glctx)
{

    int align = 4;

    static const char *shaderHeader =
#if defined NVG_GL_VERSION_GL2
        "#define NVG_GL_VERSION_GL2 1\n"
#elif defined NVG_GL_VERSION_GL3
        "#version 150 core\n"
        "#define NVG_GL_VERSION_GL3 1\n"
#elif defined NVG_GL_VERSION_GLES2
        "#version 100\n"
        "#define NVG_GL_VERSION_GL2 1\n"
#elif defined NVG_GL_VERSION_GLES3
        "#version 300 es\n"
        "#define NVG_GL_VERSION_GL3 1\n"
#endif

#if NVG_GL_USEUNIFORMBUFFER
    "#define USE_UNIFORMBUFFER 1\n"
#else
    "#define UNIFORMARRAYSIZE 11\n"
#endif
    "\n";

    static const char *fillVertShader =
        "#ifdef NVG_GL_VERSION_GL3\n"
        "    uniform vec2 viewSize;\n"
        "    in vec2 vertex;\n"
        "    in vec2 tcoord;\n"
        "    out vec2 ftcoord;\n"
        "    out vec2 fpos;\n"
        "#else\n"
        "    uniform vec2 viewSize;\n"
        "    attribute vec2 vertex;\n"
        "    attribute vec2 tcoord;\n"
        "    varying vec2 ftcoord;\n"
        "    varying vec2 fpos;\n"
        "#endif\n"
        "void main(void) {\n"
        "    ftcoord = tcoord;\n"
        "    fpos = vertex;\n"
        "    gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - 2.0*vertex.y/viewSize.y, 0, 1);\n"
        "}\n";

    static const char *fillFragShader =
        "#ifdef GL_ES\n"
        "#if defined(GL_FRAGMENT_PRECISION_HIGH) || defined(NVG_GL_VERSION_GL3)\n"
        " precision highp float;\n"
        "#else\n"
        " precision mediump float;\n"
        "#endif\n"
        "#endif\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "#ifdef USE_UNIFORMBUFFER\n"
        "    layout(std140) uniform frag {\n"
        "        mat3 scissorMat;\n"
        "        mat3 paintMat;\n"
        "        vec4 innerCol;\n"
        "        vec4 outerCol;\n"
        "        vec2 scissorExt;\n"
        "        vec2 scissorScale;\n"
        "        vec2 extent;\n"
        "        float radius;\n"
        "        float feather;\n"
        "        float strokeMult;\n"
        "        float strokeThr;\n"
        "        int texType;\n"
        "        int type;\n"
        "    };\n"
        "#else\n"
        "    uniform vec4 frag[UNIFORMARRAYSIZE];\n"
        "#endif\n"
        "    uniform sampler2D tex;\n"
        "    in vec2 ftcoord;\n"
        "    in vec2 fpos;\n"
        "    out vec4 outColor;\n"
        "#else\n"
        "    uniform vec4 frag[UNIFORMARRAYSIZE];\n"
        "    uniform sampler2D tex;\n"
        "    varying vec2 ftcoord;\n"
        "    varying vec2 fpos;\n"
        "#endif\n"
        "#ifndef USE_UNIFORMBUFFER\n"
        "    #define scissorMat mat3(frag[0].xyz, frag[1].xyz, frag[2].xyz)\n"
        "    #define paintMat mat3(frag[3].xyz, frag[4].xyz, frag[5].xyz)\n"
        "    #define innerCol frag[6]\n"
        "    #define outerCol frag[7]\n"
        "    #define scissorExt frag[8].xy\n"
        "    #define scissorScale frag[8].zw\n"
        "    #define extent frag[9].xy\n"
        "    #define radius frag[9].z\n"
        "    #define feather frag[9].w\n"
        "    #define strokeMult frag[10].x\n"
        "    #define strokeThr frag[10].y\n"
        "    #define texType int(frag[10].z)\n"
        "    #define type int(frag[10].w)\n"
        "#endif\n"
        "\n"
        "float sdroundrect(vec2 pt, vec2 ext, float rad) {\n"
        "    vec2 ext2 = ext - vec2(rad,rad);\n"
        "    vec2 d = abs(pt) - ext2;\n"
        "    return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;\n"
        "}\n"
        "\n"
        "// Scissoring\n"
        "float scissorMask(vec2 p) {\n"
        "    vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);\n"
        "    sc = vec2(0.5,0.5) - sc * scissorScale;\n"
        "    return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);\n"
        "}\n"
        "#ifdef EDGE_AA\n"
        "// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.\n"
        "float strokeMask() {\n"
        "    return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, ftcoord.y);\n"
        "}\n"
        "#endif\n"
        "\n"
        "void main(void) {\n"
        "   vec4 result;\n"
        "    float scissor = scissorMask(fpos);\n"
        "#ifdef EDGE_AA\n"
        "    float strokeAlpha = strokeMask();\n"
        "    if (strokeAlpha < strokeThr) discard;\n"
        "#else\n"
        "    float strokeAlpha = 1.0;\n"
        "#endif\n"
        "    if (type == 0) {            // Gradient\n"
        "        // Calculate gradient color using box gradient\n"
        "        vec2 pt = (paintMat * vec3(fpos,1.0)).xy;\n"
        "        float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);\n"
        "        vec4 color = mix(innerCol,outerCol,d);\n"
        "        // Combine alpha\n"
        "        color *= strokeAlpha * scissor;\n"
        "        result = color;\n"
        "    } else if (type == 1) {        // Image\n"
        "        // Calculate color fron texture\n"
        "        vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "        vec4 color = texture(tex, pt);\n"
        "#else\n"
        "        vec4 color = texture2D(tex, pt);\n"
        "#endif\n"
        "        if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
        "        if (texType == 2) color = vec4(color.x);"
        "        // Apply color tint and alpha.\n"
        "        color *= innerCol;\n"
        "        // Combine alpha\n"
        "        color *= strokeAlpha * scissor;\n"
        "        result = color;\n"
        "    } else if (type == 2) {        // Stencil fill\n"
        "        result = vec4(1,1,1,1);\n"
        "    } else if (type == 3) {        // Textured tris\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "        vec4 color = texture(tex, ftcoord);\n"
        "#else\n"
        "        vec4 color = texture2D(tex, ftcoord);\n"
        "#endif\n"
        "        if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
        "        if (texType == 2) color = vec4(color.x);"
        "        color *= scissor;\n"
        "        result = color * innerCol;\n"
        "    }\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "    outColor = result;\n"
        "#else\n"
        "    gl_FragColor = result;\n"
        "#endif\n"
        "}\n";

    checkError(glctx, "init");

    if (glctx->flags & NVG_ANTIALIAS)
    {

        if (!createShader(&glctx->shader, "shader", shaderHeader, "#define EDGE_AA 1\n", fillVertShader, fillFragShader))
            return 0;

    }

    else
    {

        if (!createShader(&glctx->shader, "shader", shaderHeader, NULL, fillVertShader, fillFragShader))
            return 0;
    }

    checkError(glctx, "uniform locations");
    getUniforms(&glctx->shader);
#if defined NVG_GL_VERSION_GL3
    glGenVertexArrays(1, &glctx->vertArr);
#endif
    glGenBuffers(1, &glctx->vertBuf);
#if NVG_GL_USEUNIFORMBUFFER
    glUniformBlockBinding(glctx->shader.prog, glctx->shader.loc[NVG_GL_LOC_FRAG], NVG_GL_FRAG_BINDING);
    glGenBuffers(1, &glctx->fragBuf);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
#endif

    glctx->fragSize = sizeof (struct nvg_gl_fraguniforms) + align - sizeof (struct nvg_gl_fraguniforms) % align;

    checkError(glctx, "create done");
    glFinish();

    return 1;

}

static int renderCreateTexture(struct nvg_gl_context *glctx, int type, int w, int h, int imageFlags, const unsigned char *data)
{

    struct nvg_gl_texture *tex = allocTexture(glctx);

    if (!tex)
        return 0;

#ifdef NVG_GL_VERSION_GLES2
    if (nearestPow2(w) != (unsigned int)w || nearestPow2(h) != (unsigned int)h)
    {

        if ((imageFlags & NVG_IMAGE_REPEATX) != 0 || (imageFlags & NVG_IMAGE_REPEATY) != 0)
        {

            printf("Repeat X/Y is not supported for non power-of-two textures (%d x %d)\n", w, h);

            imageFlags &= ~(NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);

        }

        if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
        {

            printf("Mip-maps is not support for non power-of-two textures (%d x %d)\n", w, h);

            imageFlags &= ~NVG_IMAGE_GENERATE_MIPMAPS;

        }

    }
#endif

    glGenTextures(1, &tex->tex);

    tex->width = w;
    tex->height = h;
    tex->type = type;
    tex->flags = imageFlags;

    bindTexture(glctx, tex->tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifndef NVG_GL_VERSION_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

#if defined (NVG_GL_VERSION_GL2)
    if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
#endif

    if (type == NVG_TEXTURE_RGBA)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else
#if defined(NVG_GL_VERSION_GLES2) || defined (NVG_GL_VERSION_GL2)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
#elif defined(NVG_GL_VERSION_GLES3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
#endif

    if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
    {

        if (imageFlags & NVG_IMAGE_NEAREST)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


    }

    else
    {

        if (imageFlags & NVG_IMAGE_NEAREST)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    }

    if (imageFlags & NVG_IMAGE_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (imageFlags & NVG_IMAGE_REPEATX)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    if (imageFlags & NVG_IMAGE_REPEATY)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#ifndef NVG_GL_VERSION_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

#if !defined(NVG_GL_VERSION_GL2)
    if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
        glGenerateMipmap(GL_TEXTURE_2D);
#endif

    checkError(glctx, "create tex");
    bindTexture(glctx, 0);

    return tex->id;

}

static int renderUpdateTexture(struct nvg_gl_context *glctx, int image, int x, int y, int w, int h, const unsigned char *data)
{

    struct nvg_gl_texture *tex = findTexture(glctx, image);

    if (!tex)
        return 0;

    bindTexture(glctx, tex->tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifndef NVG_GL_VERSION_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
#else
    if (tex->type == NVG_TEXTURE_RGBA)
        data += y * tex->width * 4;
    else
        data += y * tex->width;

    x = 0;
    w = tex->width;
#endif

    if (tex->type == NVG_TEXTURE_RGBA)
        glTexSubImage2D(GL_TEXTURE_2D, 0, x,y, w,h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else
#if defined(NVG_GL_VERSION_GLES2) || defined(NVG_GL_VERSION_GL2)
        glTexSubImage2D(GL_TEXTURE_2D, 0, x,y, w,h, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
#else
        glTexSubImage2D(GL_TEXTURE_2D, 0, x,y, w,h, GL_RED, GL_UNSIGNED_BYTE, data);
#endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#ifndef NVG_GL_VERSION_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif
    bindTexture(glctx, 0);

    return 1;

}

static void xformToMat3x4(float *m3, float *t)
{

    m3[0] = t[0];
    m3[1] = t[1];
    m3[2] = 0.0f;
    m3[3] = 0.0f;
    m3[4] = t[2];
    m3[5] = t[3];
    m3[6] = 0.0f;
    m3[7] = 0.0f;
    m3[8] = t[4];
    m3[9] = t[5];
    m3[10] = 1.0f;
    m3[11] = 0.0f;

}

static struct nvg_color premulColor(struct nvg_color c)
{

    c.r *= c.a;
    c.g *= c.a;
    c.b *= c.a;

    return c;

}

static int convertPaint(struct nvg_gl_context *glctx, struct nvg_gl_fraguniforms *frag, struct nvg_paint *paint, struct nvg_scissor *scissor, float width, float fringe, float strokeThr)
{

    struct nvg_gl_texture *tex = NULL;
    float invxform[6];

    memset(frag, 0, sizeof (struct nvg_gl_fraguniforms));

    frag->innerCol = premulColor(paint->innerColor);
    frag->outerCol = premulColor(paint->outerColor);

    if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f)
    {

        memset(frag->scissorMat, 0, sizeof (frag->scissorMat));

        frag->scissorExt[0] = 1.0f;
        frag->scissorExt[1] = 1.0f;
        frag->scissorScale[0] = 1.0f;
        frag->scissorScale[1] = 1.0f;

    }

    else
    {

        nvgTransformInverse(invxform, scissor->xform);
        xformToMat3x4(frag->scissorMat, invxform);

        frag->scissorExt[0] = scissor->extent[0];
        frag->scissorExt[1] = scissor->extent[1];
        frag->scissorScale[0] = sqrtf(scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]) / fringe;
        frag->scissorScale[1] = sqrtf(scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]) / fringe;

    }

    memcpy(frag->extent, paint->extent, sizeof (frag->extent));

    frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
    frag->strokeThr = strokeThr;

    if (paint->image != 0)
    {

        tex = findTexture(glctx, paint->image);

        if (!tex)
            return 0;

        if ((tex->flags & NVG_IMAGE_FLIPY) != 0)
        {

            float m1[6], m2[6];

            nvgTransformTranslate(m1, 0.0f, frag->extent[1] * 0.5f);
            nvgTransformMultiply(m1, paint->xform);
            nvgTransformScale(m2, 1.0f, -1.0f);
            nvgTransformMultiply(m2, m1);
            nvgTransformTranslate(m1, 0.0f, -frag->extent[1] * 0.5f);
            nvgTransformMultiply(m1, m2);
            nvgTransformInverse(invxform, m1);

        }

        else
        {

            nvgTransformInverse(invxform, paint->xform);

        }

        frag->type = NSVG_SHADER_FILLIMG;

#if NVG_GL_USEUNIFORMBUFFER
        if (tex->type == NVG_TEXTURE_RGBA)
            frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0 : 1;
        else
            frag->texType = 2;
#else
        if (tex->type == NVG_TEXTURE_RGBA)
            frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
        else
            frag->texType = 2.0f;
#endif

    }

    else
    {

        frag->type = NSVG_SHADER_FILLGRAD;
        frag->radius = paint->radius;
        frag->feather = paint->feather;

        nvgTransformInverse(invxform, paint->xform);

    }

    xformToMat3x4(frag->paintMat, invxform);

    return 1;

}

static struct nvg_gl_fraguniforms *fragUniformPtr(struct nvg_gl_context *glctx, int i)
{

    return (struct nvg_gl_fraguniforms *)&glctx->uniforms[i];

}

static void setUniforms(struct nvg_gl_context *glctx, int uniformOffset, int image)
{

#if NVG_GL_USEUNIFORMBUFFER
    glBindBufferRange(GL_UNIFORM_BUFFER, NVG_GL_FRAG_BINDING, glctx->fragBuf, uniformOffset, sizeof (struct nvg_gl_fraguniforms));
#else
    struct nvg_gl_fraguniforms *frag = fragUniformPtr(glctx, uniformOffset);

    glUniform4fv(glctx->shader.loc[NVG_GL_LOC_FRAG], NVG_GL_UNIFORMARRAYSIZE, &(frag->uniformArray[0][0]));
#endif

    if (image != 0)
    {

        struct nvg_gl_texture *tex = findTexture(glctx, image);

        bindTexture(glctx, tex ? tex->tex : 0);
        checkError(glctx, "tex paint tex");

    }

    else
    {

        bindTexture(glctx, 0);

    }

}

static void fill(struct nvg_gl_context *glctx, struct nvg_gl_call *call)
{

    struct nvg_gl_path *paths = &glctx->paths[call->pathOffset];
    int i;

    glEnable(GL_STENCIL_TEST);
    stencilMask(glctx, 0xff);
    stencilFunc(glctx, GL_ALWAYS, 0, 0xff);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    setUniforms(glctx, call->uniformOffset, 0);
    checkError(glctx, "fill simple");
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    glDisable(GL_CULL_FACE);

    for (i = 0; i < call->pathCount; i++)
        glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);

    glEnable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    setUniforms(glctx, call->uniformOffset + glctx->fragSize, call->image);
    checkError(glctx, "fill fill");

    if (glctx->flags & NVG_ANTIALIAS)
    {

        stencilFunc(glctx, GL_EQUAL, 0x00, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        for (i = 0; i < call->pathCount; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

    }

    stencilFunc(glctx, GL_NOTEQUAL, 0x0, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    glDrawArrays(GL_TRIANGLE_STRIP, call->triangleOffset, call->triangleCount);
    glDisable(GL_STENCIL_TEST);

}

static void convexFill(struct nvg_gl_context *glctx, struct nvg_gl_call *call)
{

    struct nvg_gl_path *paths = &glctx->paths[call->pathOffset];
    int i;

    setUniforms(glctx, call->uniformOffset, call->image);
    checkError(glctx, "convex fill");

    for (i = 0; i < call->pathCount; i++)
    {

        glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);

        if (paths[i].strokeCount > 0)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

    }

}

static void stroke(struct nvg_gl_context *glctx, struct nvg_gl_call *call)
{

    struct nvg_gl_path *paths = &glctx->paths[call->pathOffset];
    int i;

    if (glctx->flags & NVG_STENCIL_STROKES)
    {

        glEnable(GL_STENCIL_TEST);
        stencilMask(glctx, 0xff);
        stencilFunc(glctx, GL_EQUAL, 0x0, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        setUniforms(glctx, call->uniformOffset + glctx->fragSize, call->image);
        checkError(glctx, "stroke fill 0");

        for (i = 0; i < call->pathCount; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

        setUniforms(glctx, call->uniformOffset, call->image);
        stencilFunc(glctx, GL_EQUAL, 0x00, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        for (i = 0; i < call->pathCount; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        stencilFunc(glctx, GL_ALWAYS, 0x0, 0xff);
        glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
        checkError(glctx, "stroke fill 1");

        for (i = 0; i < call->pathCount; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_STENCIL_TEST);

    }

    else
    {

        setUniforms(glctx, call->uniformOffset, call->image);
        checkError(glctx, "stroke fill");

        for (i = 0; i < call->pathCount; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

    }

}

static void triangles(struct nvg_gl_context *glctx, struct nvg_gl_call *call)
{

    setUniforms(glctx, call->uniformOffset, call->image);
    checkError(glctx, "triangles fill");
    glDrawArrays(GL_TRIANGLES, call->triangleOffset, call->triangleCount);

}

static struct nvg_gl_blend blendCompositeOperation(struct nvg_compositeoperationstate op)
{

    struct nvg_gl_blend blend;
    
    blend.srcRGB = op.srcRGB;
    blend.dstRGB = op.dstRGB;
    blend.srcAlpha = op.srcAlpha;
    blend.dstAlpha = op.dstAlpha;

    if (blend.srcRGB == GL_INVALID_ENUM || blend.dstRGB == GL_INVALID_ENUM || blend.srcAlpha == GL_INVALID_ENUM || blend.dstAlpha == GL_INVALID_ENUM)
    {

        blend.srcRGB = GL_ONE;
        blend.dstRGB = GL_ONE_MINUS_SRC_ALPHA;
        blend.srcAlpha = GL_ONE;
        blend.dstAlpha = GL_ONE_MINUS_SRC_ALPHA;

    }

    return blend;

}

void nvg_gl_flush(struct nvg_gl_context *glctx)
{

    int i;

    if (glctx->ncalls > 0)
    {

        glUseProgram(glctx->shader.prog);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilMask(0xffffffff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
#if NVG_GL_USESTATEFILTER
        glctx->boundTexture = 0;
        glctx->stencilMask = 0xffffffff;
        glctx->stencilFunc = GL_ALWAYS;
        glctx->stencilFuncRef = 0;
        glctx->stencilFuncMask = 0xffffffff;
        glctx->blendFunc.srcRGB = GL_INVALID_ENUM;
        glctx->blendFunc.srcAlpha = GL_INVALID_ENUM;
        glctx->blendFunc.dstRGB = GL_INVALID_ENUM;
        glctx->blendFunc.dstAlpha = GL_INVALID_ENUM;
#endif
#if NVG_GL_USEUNIFORMBUFFER
        glBindBuffer(GL_UNIFORM_BUFFER, glctx->fragBuf);
        glBufferData(GL_UNIFORM_BUFFER, glctx->nuniforms * glctx->fragSize, glctx->uniforms, GL_STREAM_DRAW);
#endif
#if defined NVG_GL_VERSION_GL3
        glBindVertexArray(glctx->vertArr);
#endif
        glBindBuffer(GL_ARRAY_BUFFER, glctx->vertBuf);
        glBufferData(GL_ARRAY_BUFFER, glctx->nverts * sizeof (struct nvg_vertex), glctx->verts, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (struct nvg_vertex), (const GLvoid *)(size_t)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (struct nvg_vertex), (const GLvoid *)(0 + 2 * sizeof (float)));
        glUniform1i(glctx->shader.loc[NVG_GL_LOC_TEX], 0);
        glUniform2fv(glctx->shader.loc[NVG_GL_LOC_VIEWSIZE], 1, glctx->view);
#if NVG_GL_USEUNIFORMBUFFER
        glBindBuffer(GL_UNIFORM_BUFFER, glctx->fragBuf);
#endif

        for (i = 0; i < glctx->ncalls; i++)
        {

            struct nvg_gl_call *call = &glctx->calls[i];

            blendFuncSeparate(glctx, &call->blendFunc);

            if (call->type == NVG_GL_FILL)
                fill(glctx, call);
            else if (call->type == NVG_GL_CONVEXFILL)
                convexFill(glctx, call);
            else if (call->type == NVG_GL_STROKE)
                stroke(glctx, call);
            else if (call->type == NVG_GL_TRIANGLES)
                triangles(glctx, call);

        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
#if defined NVG_GL_VERSION_GL3
        glBindVertexArray(0);
#endif
        glDisable(GL_CULL_FACE);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        bindTexture(glctx, 0);

    }

    glctx->nverts = 0;
    glctx->npaths = 0;
    glctx->ncalls = 0;
    glctx->nuniforms = 0;

}

static int maxVertCount(const struct nvg_path *paths, int npaths)
{

    int count = 0;
    int i;

    for (i = 0; i < npaths; i++)
    {

        count += paths[i].nfill;
        count += paths[i].nstroke;

    }

    return count;

}

static struct nvg_gl_call *allocCall(struct nvg_gl_context *glctx)
{

    struct nvg_gl_call *ret;

    if (glctx->ncalls + 1 > glctx->ccalls)
    {

        int ccalls = maxi(glctx->ncalls + 1, 128) + glctx->ccalls / 2;
        struct nvg_gl_call *calls = realloc(glctx->calls, sizeof (struct nvg_gl_call) * ccalls);

        if (!calls)
            return 0;

        glctx->calls = calls;
        glctx->ccalls = ccalls;

    }

    ret = &glctx->calls[glctx->ncalls++];

    memset(ret, 0, sizeof (struct nvg_gl_call));

    return ret;

}

static int allocPaths(struct nvg_gl_context *glctx, int n)
{

    int ret = 0;

    if (glctx->npaths + n > glctx->cpaths)
    {

        int cpaths = maxi(glctx->npaths + n, 128) + glctx->cpaths / 2;
        struct nvg_gl_path *paths = realloc(glctx->paths, sizeof (struct nvg_gl_path) * cpaths);

        if (!paths)
            return -1;

        glctx->paths = paths;
        glctx->cpaths = cpaths;

    }

    ret = glctx->npaths;
    glctx->npaths += n;

    return ret;

}

static int allocVerts(struct nvg_gl_context *glctx, int n)
{

    int ret = 0;

    if (glctx->nverts + n > glctx->cverts)
    {

        int cverts = maxi(glctx->nverts + n, 4096) + glctx->cverts / 2;
        struct nvg_vertex *verts = realloc(glctx->verts, sizeof (struct nvg_vertex) * cverts);

        if (!verts)
            return -1;

        glctx->verts = verts;
        glctx->cverts = cverts;

    }

    ret = glctx->nverts;
    glctx->nverts += n;

    return ret;

}

static int allocFragUniforms(struct nvg_gl_context *glctx, int n)
{

    int ret = 0, structSize = glctx->fragSize;

    if (glctx->nuniforms + n > glctx->cuniforms)
    {

        int cuniforms = maxi(glctx->nuniforms + n, 128) + glctx->cuniforms / 2;
        unsigned char *uniforms = realloc(glctx->uniforms, structSize * cuniforms);

        if (!uniforms)
            return -1;

        glctx->uniforms = uniforms;
        glctx->cuniforms = cuniforms;

    }

    ret = glctx->nuniforms * structSize;
    glctx->nuniforms += n;

    return ret;

}

static void vset(struct nvg_vertex *vtx, float x, float y, float u, float v)
{

    vtx->x = x;
    vtx->y = y;
    vtx->u = u;
    vtx->v = v;

}

static void renderFill(struct nvg_gl_context *glctx, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, float fringe, const float *bounds, const struct nvg_path *paths, int npaths)
{

    struct nvg_gl_call *call = allocCall(glctx);
    struct nvg_vertex *quad;
    struct nvg_gl_fraguniforms *frag;
    int i, maxverts, offset;

    if (!call)
        return;

    call->type = NVG_GL_FILL;
    call->triangleCount = 4;
    call->pathOffset = allocPaths(glctx, npaths);

    if (call->pathOffset == -1)
        goto error;

    call->pathCount = npaths;
    call->image = paint->image;
    call->blendFunc = blendCompositeOperation(compositeoperation);

    if (npaths == 1 && paths[0].convex)
    {

        call->type = NVG_GL_CONVEXFILL;
        call->triangleCount = 0;

    }

    maxverts = maxVertCount(paths, npaths) + call->triangleCount;
    offset = allocVerts(glctx, maxverts);

    if (offset == -1)
        goto error;

    for (i = 0; i < npaths; i++)
    {

        struct nvg_gl_path *copy = &glctx->paths[call->pathOffset + i];
        const struct nvg_path *path = &paths[i];

        memset(copy, 0, sizeof (struct nvg_gl_path));

        if (path->nfill > 0)
        {

            copy->fillOffset = offset;
            copy->fillCount = path->nfill;

            memcpy(&glctx->verts[offset], path->fill, sizeof (struct nvg_vertex) * path->nfill);

            offset += path->nfill;

        }

        if (path->nstroke > 0)
        {

            copy->strokeOffset = offset;
            copy->strokeCount = path->nstroke;

            memcpy(&glctx->verts[offset], path->stroke, sizeof (struct nvg_vertex) * path->nstroke);

            offset += path->nstroke;

        }

    }

    if (call->type == NVG_GL_FILL)
    {

        call->triangleOffset = offset;
        quad = &glctx->verts[call->triangleOffset];

        vset(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
        vset(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
        vset(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
        vset(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);

        call->uniformOffset = allocFragUniforms(glctx, 2);

        if (call->uniformOffset == -1)
            goto error;

        frag = fragUniformPtr(glctx, call->uniformOffset);

        memset(frag, 0, sizeof (struct nvg_gl_fraguniforms));

        frag->strokeThr = -1.0f;
        frag->type = NSVG_SHADER_SIMPLE;

        convertPaint(glctx, fragUniformPtr(glctx, call->uniformOffset + glctx->fragSize), paint, scissor, fringe, fringe, -1.0f);

    }

    else
    {

        call->uniformOffset = allocFragUniforms(glctx, 1);

        if (call->uniformOffset == -1)
            goto error;

        convertPaint(glctx, fragUniformPtr(glctx, call->uniformOffset), paint, scissor, fringe, fringe, -1.0f);

    }

    return;

error:
    if (glctx->ncalls > 0)
        glctx->ncalls--;

}

static void renderStroke(struct nvg_gl_context *glctx, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, float fringe, float strokeWidth, const struct nvg_path *paths, int npaths)
{

    struct nvg_gl_call *call = allocCall(glctx);
    int i, maxverts, offset;

    if (!call)
        return;

    call->type = NVG_GL_STROKE;
    call->pathOffset = allocPaths(glctx, npaths);

    if (call->pathOffset == -1)
        goto error;

    call->pathCount = npaths;
    call->image = paint->image;
    call->blendFunc = blendCompositeOperation(compositeoperation);
    maxverts = maxVertCount(paths, npaths);
    offset = allocVerts(glctx, maxverts);

    if (offset == -1)
        goto error;

    for (i = 0; i < npaths; i++)
    {

        struct nvg_gl_path *copy = &glctx->paths[call->pathOffset + i];
        const struct nvg_path *path = &paths[i];

        memset(copy, 0, sizeof (struct nvg_gl_path));

        if (path->nstroke)
        {

            copy->strokeOffset = offset;
            copy->strokeCount = path->nstroke;

            memcpy(&glctx->verts[offset], path->stroke, sizeof (struct nvg_vertex) * path->nstroke);

            offset += path->nstroke;

        }

    }

    if (glctx->flags & NVG_STENCIL_STROKES)
    {

        call->uniformOffset = allocFragUniforms(glctx, 2);

        if (call->uniformOffset == -1)
            goto error;

        convertPaint(glctx, fragUniformPtr(glctx, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);
        convertPaint(glctx, fragUniformPtr(glctx, call->uniformOffset + glctx->fragSize), paint, scissor, strokeWidth, fringe, 1.0f - 0.5f/255.0f);

    }

    else
    {

        call->uniformOffset = allocFragUniforms(glctx, 1);

        if (call->uniformOffset == -1)
            goto error;

        convertPaint(glctx, fragUniformPtr(glctx, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);

    }

    return;

error:
    if (glctx->ncalls > 0)
        glctx->ncalls--;

}

static void renderTriangles(struct nvg_gl_context *glctx, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, const struct nvg_vertex *verts, int nverts)
{

    struct nvg_gl_call *call = allocCall(glctx);
    struct nvg_gl_fraguniforms *frag;

    if (!call)
        return;

    call->type = NVG_GL_TRIANGLES;
    call->image = paint->image;
    call->blendFunc = blendCompositeOperation(compositeoperation);
    call->triangleOffset = allocVerts(glctx, nverts);

    if (call->triangleOffset == -1)
        goto error;

    call->triangleCount = nverts;

    memcpy(&glctx->verts[call->triangleOffset], verts, sizeof (struct nvg_vertex) * nverts);

    call->uniformOffset = allocFragUniforms(glctx, 1);

    if (call->uniformOffset == -1)
        goto error;

    frag = fragUniformPtr(glctx, call->uniformOffset);

    convertPaint(glctx, frag, paint, scissor, 1.0f, 1.0f, -1.0f);

    frag->type = NSVG_SHADER_IMG;

    return;

error:
    if (glctx->ncalls > 0)
        glctx->ncalls--;

}

static void renderDelete(struct nvg_gl_context *glctx)
{

    int i;

    deleteShader(&glctx->shader);

#if NVG_GL_VERSION_GL3
#if NVG_GL_USEUNIFORMBUFFER
    if (glctx->fragBuf != 0)
        glDeleteBuffers(1, &glctx->fragBuf);
#endif
    if (glctx->vertArr != 0)
        glDeleteVertexArrays(1, &glctx->vertArr);
#endif
    if (glctx->vertBuf != 0)
        glDeleteBuffers(1, &glctx->vertBuf);

    for (i = 0; i < glctx->ntextures; i++)
    {

        if (glctx->textures[i].tex != 0 && (glctx->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
            glDeleteTextures(1, &glctx->textures[i].tex);

    }

    free(glctx->textures);
    free(glctx->paths);
    free(glctx->verts);
    free(glctx->uniforms);
    free(glctx->calls);

}

void nvg_gl_beginframe(struct nvg_gl_context *glctx, struct nvg_context *ctx, float windowWidth, float windowHeight)
{

    ctx->tessTol = 0.25f;
    ctx->distTol = 0.01f;
    ctx->fringeWidth = 1.0f;
    ctx->drawCallCount = 0;
    ctx->fillTriCount = 0;
    ctx->strokeTriCount = 0;
    ctx->textTriCount = 0;
    glctx->view[0] = windowWidth;
    glctx->view[1] = windowHeight;

    nvg_init(ctx);
    nvg_gl_compop(ctx, NVG_SOURCE_OVER);

}

void nvg_gl_cancelframe(struct nvg_gl_context *glctx, struct nvg_context *ctx)
{

    glctx->nverts = 0;
    glctx->npaths = 0;
    glctx->ncalls = 0;
    glctx->nuniforms = 0;

}

void nvg_gl_endframe(struct nvg_gl_context *glctx, struct nvg_context *ctx)
{

    if (glctx->fontImageIdx != 0)
    {

        int fontImage = glctx->fontImages[glctx->fontImageIdx];
        struct nvg_gl_texture *t1;
        int i, j;

        if (!fontImage)
            return;

        t1 = findTexture(glctx, fontImage);

        for (i = j = 0; i < glctx->fontImageIdx; i++)
        {

            if (glctx->fontImages[i] != 0)
            {

                struct nvg_gl_texture *t2 = findTexture(glctx, glctx->fontImages[i]);

                if (t2->width < t1->width || t2->height < t1->height)
                    nvg_gl_deletetexture(glctx, glctx->fontImages[i]);
                else
                    glctx->fontImages[j++] = glctx->fontImages[i];

            }

        }

        glctx->fontImages[j++] = glctx->fontImages[0];
        glctx->fontImages[0] = fontImage;
        glctx->fontImageIdx = 0;

        for (i = j; i < NVG_GL_MAXFONTIMAGES; i++)
            glctx->fontImages[i] = 0;

    }

}

int nvg_gl_createimagefile(struct nvg_gl_context *glctx, struct nvg_context *ctx, const char *filename, int imageFlags)
{

    int w, h, n, image;
    unsigned char *img;

    stbi_set_unpremultiply_on_load(1);
    stbi_convert_iphone_png_to_rgb(1);

    img = stbi_load(filename, &w, &h, &n, 4);

    if (!img)
        return 0;

    image = nvg_gl_createimagergba(glctx, ctx, w, h, imageFlags, img);

    stbi_image_free(img);

    return image;

}

int nvg_gl_createimagemem(struct nvg_gl_context *glctx, struct nvg_context *ctx, int imageFlags, unsigned char *data, int ndata)
{

    int w, h, n, image;
    unsigned char *img = stbi_load_from_memory(data, ndata, &w, &h, &n, 4);

    if (!img)
        return 0;

    image = nvg_gl_createimagergba(glctx, ctx, w, h, imageFlags, img);

    stbi_image_free(img);

    return image;

}

int nvg_gl_createimagergba(struct nvg_gl_context *glctx, struct nvg_context *ctx, int w, int h, int imageFlags, const unsigned char *data)
{

    return renderCreateTexture(glctx, NVG_TEXTURE_RGBA, w, h, imageFlags, data);

}

void nvg_gl_updateimage(struct nvg_gl_context *glctx, int image, const unsigned char *data)
{

    struct nvg_gl_texture *tex = findTexture(glctx, image);

    renderUpdateTexture(glctx, image, 0, 0, tex->width, tex->height, data);

}

void nvg_gl_imagesize(struct nvg_gl_context *glctx, int image, int *w, int *h)
{

    struct nvg_gl_texture *tex = findTexture(glctx, image);

    *w = tex->width;
    *h = tex->height;

}

void nvg_gl_deletetexture(struct nvg_gl_context *glctx, int id)
{

    struct nvg_gl_texture *texture = findTexture(glctx, id);

    if (texture)
    {

        if (texture->tex != 0 && (texture->flags & NVG_IMAGE_NODELETE) == 0)
            glDeleteTextures(1, &texture->tex);

        memset(texture, 0, sizeof (struct nvg_gl_texture));

    }

}

void nvg_gl_fill(struct nvg_gl_context *glctx, struct nvg_context *ctx, struct nvg_state *state)
{

    unsigned int i;

    nvg_flatten_paths(ctx);

    if (glctx->edgeAntiAlias && state->shapeAntiAlias)
        nvg_expand_fill(ctx, ctx->fringeWidth, NVG_MITER, 2.4f);
    else
        nvg_expand_fill(ctx, 0.0f, NVG_MITER, 2.4f);

    renderFill(glctx, &state->fill, state->compositeoperation, &state->scissor, ctx->fringeWidth, ctx->cache.bounds, ctx->cache.paths, ctx->cache.npaths);

    for (i = 0; i < ctx->cache.npaths; i++)
    {

        const struct nvg_path *path = &ctx->cache.paths[i];

        ctx->fillTriCount += path->nfill - 2;
        ctx->fillTriCount += path->nstroke - 2;
        ctx->drawCallCount += 2;

    }

}

void nvg_gl_stroke(struct nvg_gl_context *glctx, struct nvg_context *ctx, struct nvg_state *state)
{

    float strokeWidth = clampf(state->strokeWidth, 0.0f, 200.0f);
    unsigned int i;

    if (strokeWidth < ctx->fringeWidth)
    {

        float alpha = clampf(strokeWidth / ctx->fringeWidth, 0.0f, 1.0f);

        state->stroke.innerColor.a *= alpha * alpha;
        state->stroke.outerColor.a *= alpha * alpha;
        strokeWidth = ctx->fringeWidth;

    }

    nvg_flatten_paths(ctx);

    if (glctx->edgeAntiAlias && state->shapeAntiAlias)
        nvg_expand_stroke(ctx, strokeWidth * 0.5f, ctx->fringeWidth, state->linecap, state->linejoin, state->miterLimit);
    else
        nvg_expand_stroke(ctx, strokeWidth * 0.5f, 0.0f, state->linecap, state->linejoin, state->miterLimit);

    renderStroke(glctx, &state->stroke, state->compositeoperation, &state->scissor, ctx->fringeWidth, strokeWidth, ctx->cache.paths, ctx->cache.npaths);

    for (i = 0; i < ctx->cache.npaths; i++)
    {

        const struct nvg_path *path = &ctx->cache.paths[i];

        ctx->strokeTriCount += path->nstroke - 2;
        ctx->drawCallCount++;

    }
    
}

float nvg_gl_text(struct nvg_gl_context *glctx, struct nvg_context *ctx, struct nvg_state *state, struct fons_context *fsctx, float x, float y, const char *string, const char *end)
{

    struct fons_textiter iter;
    struct fons_quad q;
    struct nvg_vertex *verts = ctx->cache.verts;
    int cverts = 0;
    int nverts = 0;
    int dirty[4];

    cverts = maxi(2, (int)(end - string)) * 6;

    fonsTextIterInit(fsctx, &iter, x, y, string, end, FONS_GLYPH_BITMAP_REQUIRED);

    while (fonsTextIterNext(fsctx, &iter, &q))
    {

        float c[4 * 2];

        nvgTransformPoint(&c[0], &c[1], state->xform, q.x0, q.y0);
        nvgTransformPoint(&c[2], &c[3], state->xform, q.x1, q.y0);
        nvgTransformPoint(&c[4], &c[5], state->xform, q.x1, q.y1);
        nvgTransformPoint(&c[6], &c[7], state->xform, q.x0, q.y1);

        if (nverts + 6 <= cverts)
        {

            vset(&verts[nverts + 0], c[0], c[1], q.s0, q.t0);
            vset(&verts[nverts + 1], c[4], c[5], q.s1, q.t1);
            vset(&verts[nverts + 2], c[2], c[3], q.s1, q.t0);
            vset(&verts[nverts + 3], c[0], c[1], q.s0, q.t0);
            vset(&verts[nverts + 4], c[6], c[7], q.s0, q.t1);
            vset(&verts[nverts + 5], c[4], c[5], q.s1, q.t1);

            nverts += 6;

        }

    }

    if (fonsValidateTexture(fsctx, dirty))
        renderUpdateTexture(glctx, glctx->fontImages[glctx->fontImageIdx], dirty[0], dirty[1], dirty[2] - dirty[0], dirty[3] - dirty[1], fsctx->texData);

    state->fill.image = glctx->fontImages[glctx->fontImageIdx];

    renderTriangles(glctx, &state->fill, state->compositeoperation, &state->scissor, verts, nverts);

    ctx->drawCallCount++;
    ctx->textTriCount += nverts / 3;

    return iter.nextx;

}

void nvg_gl_create(struct nvg_gl_context *glctx, int w, int h, int flags)
{

    glctx->flags = flags;
    glctx->edgeAntiAlias = flags & NVG_ANTIALIAS ? 1 : 0;

    renderCreate(glctx);

    glctx->fontImages[0] = renderCreateTexture(glctx, NVG_TEXTURE_ALPHA, w, h, 0, NULL);

}

void nvg_gl_delete(struct nvg_gl_context *glctx)
{

    int i;

    for (i = 0; i < NVG_GL_MAXFONTIMAGES; i++)
    {

        if (glctx->fontImages[i] != 0)
        {

            nvg_gl_deletetexture(glctx, glctx->fontImages[i]);

            glctx->fontImages[i] = 0;

        }

    }

    renderDelete(glctx);

}

void nvg_gl_compop(struct nvg_context *ctx, int op)
{

    int sfactor, dfactor;

    if (op == NVG_SOURCE_OVER)
    {

        sfactor = GL_ONE;
        dfactor = GL_ONE_MINUS_SRC_ALPHA;

    }

    else if (op == NVG_SOURCE_IN)
    {

        sfactor = GL_DST_ALPHA;
        dfactor = GL_ZERO;

    }

    else if (op == NVG_SOURCE_OUT)
    {

        sfactor = GL_ONE_MINUS_DST_ALPHA;
        dfactor = GL_ZERO;

    }

    else if (op == NVG_ATOP)
    {

        sfactor = GL_DST_ALPHA;
        dfactor = GL_ONE_MINUS_SRC_ALPHA;

    }

    else if (op == NVG_DESTINATION_OVER)
    {

        sfactor = GL_ONE_MINUS_DST_ALPHA;
        dfactor = GL_ONE;

    }

    else if (op == NVG_DESTINATION_IN)
    {

        sfactor = GL_ZERO;
        dfactor = GL_SRC_ALPHA;

    }

    else if (op == NVG_DESTINATION_OUT)
    {

        sfactor = GL_ZERO;
        dfactor = GL_ONE_MINUS_SRC_ALPHA;

    }

    else if (op == NVG_DESTINATION_ATOP)
    {

        sfactor = GL_ONE_MINUS_DST_ALPHA;
        dfactor = GL_SRC_ALPHA;

    }

    else if (op == NVG_LIGHTER)
    {

        sfactor = GL_ONE;
        dfactor = GL_ONE;

    }

    else if (op == NVG_COPY)
    {

        sfactor = GL_ONE;
        dfactor = GL_ZERO;

    }

    else if (op == NVG_XOR)
    {

        sfactor = GL_ONE_MINUS_DST_ALPHA;
        dfactor = GL_ONE_MINUS_SRC_ALPHA;

    }

    else
    {

        sfactor = GL_ONE;
        dfactor = GL_ZERO;

    }

    ctx->state.compositeoperation.srcRGB = sfactor;
    ctx->state.compositeoperation.dstRGB = dfactor;
    ctx->state.compositeoperation.srcAlpha = sfactor;
    ctx->state.compositeoperation.dstAlpha = dfactor;

}

