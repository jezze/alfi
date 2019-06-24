#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "nvg.h"
#include "nvg_gl.h"

static int maxi(int a, int b)
{

    return a > b ? a : b;

}

#ifdef NANOVG_GLES2
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

static void bindTexture(struct nvg_gl_context *gl, GLuint tex)
{

#if NANOVG_GL_USE_STATE_FILTER
    if (gl->boundTexture != tex)
    {

        gl->boundTexture = tex;
        glBindTexture(GL_TEXTURE_2D, tex);

    }
#else
    glBindTexture(GL_TEXTURE_2D, tex);
#endif

}

static void stencilMask(struct nvg_gl_context *gl, GLuint mask)
{

#if NANOVG_GL_USE_STATE_FILTER
    if (gl->stencilMask != mask)
    {

        gl->stencilMask = mask;
        glStencilMask(mask);

    }
#else
    glStencilMask(mask);
#endif

}

static void stencilFunc(struct nvg_gl_context *gl, GLenum func, GLint ref, GLuint mask)
{

#if NANOVG_GL_USE_STATE_FILTER
    if ((gl->stencilFunc != func) || (gl->stencilFuncRef != ref) || (gl->stencilFuncMask != mask))
    {

        gl->stencilFunc = func;
        gl->stencilFuncRef = ref;
        gl->stencilFuncMask = mask;
        glStencilFunc(func, ref, mask);

    }
#else
    glStencilFunc(func, ref, mask);
#endif

}

static void blendFuncSeparate(struct nvg_gl_context *gl, const struct nvg_gl_blend *blend)
{

#if NANOVG_GL_USE_STATE_FILTER
    if ((gl->blendFunc.srcRGB != blend->srcRGB) || (gl->blendFunc.dstRGB != blend->dstRGB) || (gl->blendFunc.srcAlpha != blend->srcAlpha) || (gl->blendFunc.dstAlpha != blend->dstAlpha))
    {

        gl->blendFunc = *blend;

        glBlendFuncSeparate(blend->srcRGB, blend->dstRGB, blend->srcAlpha,blend->dstAlpha);

    }
#else
    glBlendFuncSeparate(blend->srcRGB, blend->dstRGB, blend->srcAlpha,blend->dstAlpha);
#endif

}

static struct nvg_gl_texture *allocTexture(struct nvg_gl_context *gl)
{

    struct nvg_gl_texture *tex = NULL;
    int i;

    for (i = 0; i < gl->ntextures; i++)
    {

        if (gl->textures[i].id == 0)
        {

            tex = &gl->textures[i];

            break;

        }

    }

    if (tex == NULL)
    {

        if (gl->ntextures + 1 > gl->ctextures)
        {

            int ctextures = maxi(gl->ntextures + 1, 4) +  gl->ctextures / 2;
            struct nvg_gl_texture *textures = realloc(gl->textures, sizeof (struct nvg_gl_texture) * ctextures);

            if (textures == NULL)
                return NULL;

            gl->textures = textures;
            gl->ctextures = ctextures;

        }

        tex = &gl->textures[gl->ntextures++];

    }

    memset(tex, 0, sizeof (struct nvg_gl_texture));

    tex->id = ++gl->textureId;

    return tex;

}

static struct nvg_gl_texture *findTexture(struct nvg_gl_context *gl, int id)
{

    int i;

    for (i = 0; i < gl->ntextures; i++)
    {

        if (gl->textures[i].id == id)
            return &gl->textures[i];

    }

    return NULL;

}

static int deleteTexture(struct nvg_gl_context *gl, int id)
{

    int i;

    for (i = 0; i < gl->ntextures; i++)
    {

        if (gl->textures[i].id == id)
        {

            if (gl->textures[i].tex != 0 && (gl->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
                glDeleteTextures(1, &gl->textures[i].tex);

            memset(&gl->textures[i], 0, sizeof (gl->textures[i]));

            return 1;

        }

    }

    return 0;

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

static void checkError(struct nvg_gl_context *gl, const char *str)
{

    GLenum err;

    if ((gl->flags & NVG_DEBUG) == 0)
        return;

    err = glGetError();

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
    str[1] = opts != NULL ? opts : "";

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

    if (shader->prog != 0)
        glDeleteProgram(shader->prog);

    if (shader->vert != 0)
        glDeleteShader(shader->vert);

    if (shader->frag != 0)
        glDeleteShader(shader->frag);

}

static void getUniforms(struct nvg_gl_shader *shader)
{

    shader->loc[GLNVG_LOC_VIEWSIZE] = glGetUniformLocation(shader->prog, "viewSize");
    shader->loc[GLNVG_LOC_TEX] = glGetUniformLocation(shader->prog, "tex");
#if NANOVG_GL_USE_UNIFORMBUFFER
    shader->loc[GLNVG_LOC_FRAG] = glGetUniformBlockIndex(shader->prog, "frag");
#else
    shader->loc[GLNVG_LOC_FRAG] = glGetUniformLocation(shader->prog, "frag");
#endif

}

static int renderCreate(void *uptr)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    int align = 4;

    static const char *shaderHeader =
#if defined NANOVG_GL2
        "#define NANOVG_GL2 1\n"
#elif defined NANOVG_GL3
        "#version 150 core\n"
        "#define NANOVG_GL3 1\n"
#elif defined NANOVG_GLES2
        "#version 100\n"
        "#define NANOVG_GL2 1\n"
#elif defined NANOVG_GLES3
        "#version 300 es\n"
        "#define NANOVG_GL3 1\n"
#endif

#if NANOVG_GL_USE_UNIFORMBUFFER
    "#define USE_UNIFORMBUFFER 1\n"
#else
    "#define UNIFORMARRAY_SIZE 11\n"
#endif
    "\n";

    static const char *fillVertShader =
        "#ifdef NANOVG_GL3\n"
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
        "#if defined(GL_FRAGMENT_PRECISION_HIGH) || defined(NANOVG_GL3)\n"
        " precision highp float;\n"
        "#else\n"
        " precision mediump float;\n"
        "#endif\n"
        "#endif\n"
        "#ifdef NANOVG_GL3\n"
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
        "    uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
        "#endif\n"
        "    uniform sampler2D tex;\n"
        "    in vec2 ftcoord;\n"
        "    in vec2 fpos;\n"
        "    out vec4 outColor;\n"
        "#else\n"
        "    uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
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
        "#ifdef NANOVG_GL3\n"
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
        "#ifdef NANOVG_GL3\n"
        "        vec4 color = texture(tex, ftcoord);\n"
        "#else\n"
        "        vec4 color = texture2D(tex, ftcoord);\n"
        "#endif\n"
        "        if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
        "        if (texType == 2) color = vec4(color.x);"
        "        color *= scissor;\n"
        "        result = color * innerCol;\n"
        "    }\n"
        "#ifdef NANOVG_GL3\n"
        "    outColor = result;\n"
        "#else\n"
        "    gl_FragColor = result;\n"
        "#endif\n"
        "}\n";

    checkError(gl, "init");

    if (gl->flags & NVG_ANTIALIAS)
    {

        if (createShader(&gl->shader, "shader", shaderHeader, "#define EDGE_AA 1\n", fillVertShader, fillFragShader) == 0)
            return 0;

    }

    else
    {

        if (createShader(&gl->shader, "shader", shaderHeader, NULL, fillVertShader, fillFragShader) == 0)
            return 0;
    }

    checkError(gl, "uniform locations");
    getUniforms(&gl->shader);
#if defined NANOVG_GL3
    glGenVertexArrays(1, &gl->vertArr);
#endif
    glGenBuffers(1, &gl->vertBuf);
#if NANOVG_GL_USE_UNIFORMBUFFER
    glUniformBlockBinding(gl->shader.prog, gl->shader.loc[GLNVG_LOC_FRAG], GLNVG_FRAG_BINDING);
    glGenBuffers(1, &gl->fragBuf);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
#endif

    gl->fragSize = sizeof (struct nvg_gl_fraguniforms) + align - sizeof (struct nvg_gl_fraguniforms) % align;

    checkError(gl, "create done");
    glFinish();

    return 1;

}

static int renderCreateTexture(void *uptr, int type, int w, int h, int imageFlags, const unsigned char *data)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    struct nvg_gl_texture *tex = allocTexture(gl);

    if (tex == NULL)
        return 0;

#ifdef NANOVG_GLES2
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

    bindTexture(gl, tex->tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
#ifndef NANOVG_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

#if defined (NANOVG_GL2)
    if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
#endif

    if (type == NVG_TEXTURE_RGBA)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else
#if defined(NANOVG_GLES2) || defined (NANOVG_GL2)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
#elif defined(NANOVG_GLES3)
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
#ifndef NANOVG_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

#if !defined(NANOVG_GL2)
    if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
        glGenerateMipmap(GL_TEXTURE_2D);
#endif

    checkError(gl, "create tex");
    bindTexture(gl, 0);

    return tex->id;

}

static int renderDeleteTexture(void *uptr, int image)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;

    return deleteTexture(gl, image);

}

static int renderUpdateTexture(void *uptr, int image, int x, int y, int w, int h, const unsigned char *data)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    struct nvg_gl_texture *tex = findTexture(gl, image);

    if (tex == NULL)
        return 0;

    bindTexture(gl, tex->tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
#ifndef NANOVG_GLES2
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
#if defined(NANOVG_GLES2) || defined(NANOVG_GL2)
        glTexSubImage2D(GL_TEXTURE_2D, 0, x,y, w,h, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
#else
        glTexSubImage2D(GL_TEXTURE_2D, 0, x,y, w,h, GL_RED, GL_UNSIGNED_BYTE, data);
#endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#ifndef NANOVG_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif
    bindTexture(gl, 0);

    return 1;

}

static int renderGetTextureSize(void *uptr, int image, int *w, int *h)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    struct nvg_gl_texture *tex = findTexture(gl, image);

    if (tex == NULL)
        return 0;

    *w = tex->width;
    *h = tex->height;

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

static int convertPaint(struct nvg_gl_context *gl, struct nvg_gl_fraguniforms *frag, struct nvg_paint *paint, struct nvg_scissor *scissor, float width, float fringe, float strokeThr)
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
        frag->scissorScale[0] = sqrtf(scissor->xform[0]*scissor->xform[0] + scissor->xform[2]*scissor->xform[2]) / fringe;
        frag->scissorScale[1] = sqrtf(scissor->xform[1]*scissor->xform[1] + scissor->xform[3]*scissor->xform[3]) / fringe;

    }

    memcpy(frag->extent, paint->extent, sizeof (frag->extent));

    frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
    frag->strokeThr = strokeThr;

    if (paint->image != 0)
    {

        tex = findTexture(gl, paint->image);

        if (tex == NULL)
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

#if NANOVG_GL_USE_UNIFORMBUFFER
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

static struct nvg_gl_fraguniforms *fragUniformPtr(struct nvg_gl_context *gl, int i);

static void setUniforms(struct nvg_gl_context *gl, int uniformOffset, int image)
{

#if NANOVG_GL_USE_UNIFORMBUFFER
    glBindBufferRange(GL_UNIFORM_BUFFER, GLNVG_FRAG_BINDING, gl->fragBuf, uniformOffset, sizeof (struct nvg_gl_fraguniforms));
#else
    struct nvg_gl_fraguniforms *frag = fragUniformPtr(gl, uniformOffset);
    glUniform4fv(gl->shader.loc[GLNVG_LOC_FRAG], NANOVG_GL_UNIFORMARRAY_SIZE, &(frag->uniformArray[0][0]));
#endif

    if (image != 0)
    {

        struct nvg_gl_texture *tex = findTexture(gl, image);

        bindTexture(gl, tex != NULL ? tex->tex : 0);
        checkError(gl, "tex paint tex");

    }

    else
    {

        bindTexture(gl, 0);

    }

}

static void renderViewport(void *uptr, float width, float height, float devicePixelRatio)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;

    gl->view[0] = width;
    gl->view[1] = height;

}

static void fill(struct nvg_gl_context *gl, struct nvg_gl_call *call)
{

    struct nvg_gl_path *paths = &gl->paths[call->pathOffset];
    int i, npaths = call->pathCount;

    glEnable(GL_STENCIL_TEST);
    stencilMask(gl, 0xff);
    stencilFunc(gl, GL_ALWAYS, 0, 0xff);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    setUniforms(gl, call->uniformOffset, 0);
    checkError(gl, "fill simple");
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    glDisable(GL_CULL_FACE);

    for (i = 0; i < npaths; i++)
        glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);

    glEnable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    setUniforms(gl, call->uniformOffset + gl->fragSize, call->image);
    checkError(gl, "fill fill");

    if (gl->flags & NVG_ANTIALIAS)
    {

        stencilFunc(gl, GL_EQUAL, 0x00, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        for (i = 0; i < npaths; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);
    }

    stencilFunc(gl, GL_NOTEQUAL, 0x0, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    glDrawArrays(GL_TRIANGLE_STRIP, call->triangleOffset, call->triangleCount);
    glDisable(GL_STENCIL_TEST);

}

static void convexFill(struct nvg_gl_context *gl, struct nvg_gl_call *call)
{

    struct nvg_gl_path *paths = &gl->paths[call->pathOffset];
    int i, npaths = call->pathCount;

    setUniforms(gl, call->uniformOffset, call->image);
    checkError(gl, "convex fill");

    for (i = 0; i < npaths; i++)
    {

        glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);

        if (paths[i].strokeCount > 0)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

    }

}

static void stroke(struct nvg_gl_context *gl, struct nvg_gl_call *call)
{

    struct nvg_gl_path *paths = &gl->paths[call->pathOffset];
    int npaths = call->pathCount, i;

    if (gl->flags & NVG_STENCIL_STROKES)
    {

        glEnable(GL_STENCIL_TEST);
        stencilMask(gl, 0xff);
        stencilFunc(gl, GL_EQUAL, 0x0, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        setUniforms(gl, call->uniformOffset + gl->fragSize, call->image);
        checkError(gl, "stroke fill 0");

        for (i = 0; i < npaths; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

        setUniforms(gl, call->uniformOffset, call->image);
        stencilFunc(gl, GL_EQUAL, 0x00, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        for (i = 0; i < npaths; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        stencilFunc(gl, GL_ALWAYS, 0x0, 0xff);
        glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
        checkError(gl, "stroke fill 1");

        for (i = 0; i < npaths; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_STENCIL_TEST);

    }

    else
    {

        setUniforms(gl, call->uniformOffset, call->image);
        checkError(gl, "stroke fill");

        for (i = 0; i < npaths; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset, paths[i].strokeCount);

    }

}

static void triangles(struct nvg_gl_context *gl, struct nvg_gl_call *call)
{

    setUniforms(gl, call->uniformOffset, call->image);
    checkError(gl, "triangles fill");
    glDrawArrays(GL_TRIANGLES, call->triangleOffset, call->triangleCount);

}

static void renderCancel(void *uptr)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;

    gl->nverts = 0;
    gl->npaths = 0;
    gl->ncalls = 0;
    gl->nuniforms = 0;

}

static GLenum glnvg_convertBlendFuncFactor(int factor)
{

    if (factor == NVG_ZERO)
        return GL_ZERO;

    if (factor == NVG_ONE)
        return GL_ONE;

    if (factor == NVG_SRC_COLOR)
        return GL_SRC_COLOR;

    if (factor == NVG_ONE_MINUS_SRC_COLOR)
        return GL_ONE_MINUS_SRC_COLOR;

    if (factor == NVG_DST_COLOR)
        return GL_DST_COLOR;

    if (factor == NVG_ONE_MINUS_DST_COLOR)
        return GL_ONE_MINUS_DST_COLOR;

    if (factor == NVG_SRC_ALPHA)
        return GL_SRC_ALPHA;

    if (factor == NVG_ONE_MINUS_SRC_ALPHA)
        return GL_ONE_MINUS_SRC_ALPHA;

    if (factor == NVG_DST_ALPHA)
        return GL_DST_ALPHA;

    if (factor == NVG_ONE_MINUS_DST_ALPHA)
        return GL_ONE_MINUS_DST_ALPHA;

    if (factor == NVG_SRC_ALPHA_SATURATE)
        return GL_SRC_ALPHA_SATURATE;

    return GL_INVALID_ENUM;

}

static struct nvg_gl_blend blendCompositeOperation(struct nvg_compositeoperationstate op)
{

    struct nvg_gl_blend blend;
    
    blend.srcRGB = glnvg_convertBlendFuncFactor(op.srcRGB);
    blend.dstRGB = glnvg_convertBlendFuncFactor(op.dstRGB);
    blend.srcAlpha = glnvg_convertBlendFuncFactor(op.srcAlpha);
    blend.dstAlpha = glnvg_convertBlendFuncFactor(op.dstAlpha);

    if (blend.srcRGB == GL_INVALID_ENUM || blend.dstRGB == GL_INVALID_ENUM || blend.srcAlpha == GL_INVALID_ENUM || blend.dstAlpha == GL_INVALID_ENUM)
    {

        blend.srcRGB = GL_ONE;
        blend.dstRGB = GL_ONE_MINUS_SRC_ALPHA;
        blend.srcAlpha = GL_ONE;
        blend.dstAlpha = GL_ONE_MINUS_SRC_ALPHA;

    }

    return blend;

}

static void renderFlush(void *uptr)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    int i;

    if (gl->ncalls > 0)
    {

        glUseProgram(gl->shader.prog);
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
#if NANOVG_GL_USE_STATE_FILTER
        gl->boundTexture = 0;
        gl->stencilMask = 0xffffffff;
        gl->stencilFunc = GL_ALWAYS;
        gl->stencilFuncRef = 0;
        gl->stencilFuncMask = 0xffffffff;
        gl->blendFunc.srcRGB = GL_INVALID_ENUM;
        gl->blendFunc.srcAlpha = GL_INVALID_ENUM;
        gl->blendFunc.dstRGB = GL_INVALID_ENUM;
        gl->blendFunc.dstAlpha = GL_INVALID_ENUM;
#endif
#if NANOVG_GL_USE_UNIFORMBUFFER
        glBindBuffer(GL_UNIFORM_BUFFER, gl->fragBuf);
        glBufferData(GL_UNIFORM_BUFFER, gl->nuniforms * gl->fragSize, gl->uniforms, GL_STREAM_DRAW);
#endif
#if defined NANOVG_GL3
        glBindVertexArray(gl->vertArr);
#endif
        glBindBuffer(GL_ARRAY_BUFFER, gl->vertBuf);
        glBufferData(GL_ARRAY_BUFFER, gl->nverts * sizeof (struct nvg_vertex), gl->verts, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (struct nvg_vertex), (const GLvoid *)(size_t)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (struct nvg_vertex), (const GLvoid *)(0 + 2 * sizeof (float)));
        glUniform1i(gl->shader.loc[GLNVG_LOC_TEX], 0);
        glUniform2fv(gl->shader.loc[GLNVG_LOC_VIEWSIZE], 1, gl->view);
#if NANOVG_GL_USE_UNIFORMBUFFER
        glBindBuffer(GL_UNIFORM_BUFFER, gl->fragBuf);
#endif

        for (i = 0; i < gl->ncalls; i++)
        {

            struct nvg_gl_call *call = &gl->calls[i];

            blendFuncSeparate(gl,&call->blendFunc);

            if (call->type == GLNVG_FILL)
                fill(gl, call);
            else if (call->type == GLNVG_CONVEXFILL)
                convexFill(gl, call);
            else if (call->type == GLNVG_STROKE)
                stroke(gl, call);
            else if (call->type == GLNVG_TRIANGLES)
                triangles(gl, call);

        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
#if defined NANOVG_GL3
        glBindVertexArray(0);
#endif
        glDisable(GL_CULL_FACE);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        bindTexture(gl, 0);

    }

    gl->nverts = 0;
    gl->npaths = 0;
    gl->ncalls = 0;
    gl->nuniforms = 0;

}

static int maxVertCount(const struct nvg_path *paths, int npaths)
{

    int i, count = 0;

    for (i = 0; i < npaths; i++)
    {

        count += paths[i].nfill;
        count += paths[i].nstroke;

    }

    return count;

}

static struct nvg_gl_call *allocCall(struct nvg_gl_context *gl)
{

    struct nvg_gl_call *ret = NULL;

    if (gl->ncalls + 1 > gl->ccalls)
    {

        int ccalls = maxi(gl->ncalls + 1, 128) + gl->ccalls / 2;
        struct nvg_gl_call *calls = realloc(gl->calls, sizeof (struct nvg_gl_call) * ccalls);

        if (calls == NULL)
            return NULL;

        gl->calls = calls;
        gl->ccalls = ccalls;

    }

    ret = &gl->calls[gl->ncalls++];

    memset(ret, 0, sizeof (struct nvg_gl_call));

    return ret;

}

static int allocPaths(struct nvg_gl_context *gl, int n)
{

    int ret = 0;

    if (gl->npaths + n > gl->cpaths)
    {

        int cpaths = maxi(gl->npaths + n, 128) + gl->cpaths / 2;
        struct nvg_gl_path *paths = realloc(gl->paths, sizeof (struct nvg_gl_path) * cpaths);

        if (paths == NULL)
            return -1;

        gl->paths = paths;
        gl->cpaths = cpaths;

    }

    ret = gl->npaths;
    gl->npaths += n;

    return ret;

}

static int allocVerts(struct nvg_gl_context *gl, int n)
{

    int ret = 0;

    if (gl->nverts+n > gl->cverts)
    {

        int cverts = maxi(gl->nverts + n, 4096) + gl->cverts / 2;
        struct nvg_vertex *verts = realloc(gl->verts, sizeof (struct nvg_vertex) * cverts);

        if (verts == NULL)
            return -1;

        gl->verts = verts;
        gl->cverts = cverts;

    }

    ret = gl->nverts;
    gl->nverts += n;

    return ret;

}

static int allocFragUniforms(struct nvg_gl_context *gl, int n)
{

    int ret = 0, structSize = gl->fragSize;

    if (gl->nuniforms + n > gl->cuniforms)
    {

        int cuniforms = maxi(gl->nuniforms + n, 128) + gl->cuniforms / 2;
        unsigned char *uniforms = realloc(gl->uniforms, structSize * cuniforms);

        if (uniforms == NULL)
            return -1;

        gl->uniforms = uniforms;
        gl->cuniforms = cuniforms;

    }

    ret = gl->nuniforms * structSize;
    gl->nuniforms += n;

    return ret;

}

static struct nvg_gl_fraguniforms *fragUniformPtr(struct nvg_gl_context *gl, int i)
{

    return (struct nvg_gl_fraguniforms *)&gl->uniforms[i];

}

static void vset(struct nvg_vertex *vtx, float x, float y, float u, float v)
{

    vtx->x = x;
    vtx->y = y;
    vtx->u = u;
    vtx->v = v;

}

static void renderFill(void *uptr, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, float fringe, const float *bounds, const struct nvg_path *paths, int npaths)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    struct nvg_gl_call *call = allocCall(gl);
    struct nvg_vertex *quad;
    struct nvg_gl_fraguniforms *frag;
    int i, maxverts, offset;

    if (call == NULL)
        return;

    call->type = GLNVG_FILL;
    call->triangleCount = 4;
    call->pathOffset = allocPaths(gl, npaths);

    if (call->pathOffset == -1)
        goto error;

    call->pathCount = npaths;
    call->image = paint->image;
    call->blendFunc = blendCompositeOperation(compositeoperation);

    if (npaths == 1 && paths[0].convex)
    {

        call->type = GLNVG_CONVEXFILL;
        call->triangleCount = 0;

    }

    maxverts = maxVertCount(paths, npaths) + call->triangleCount;
    offset = allocVerts(gl, maxverts);

    if (offset == -1)
        goto error;

    for (i = 0; i < npaths; i++)
    {

        struct nvg_gl_path *copy = &gl->paths[call->pathOffset + i];
        const struct nvg_path *path = &paths[i];

        memset(copy, 0, sizeof (struct nvg_gl_path));

        if (path->nfill > 0)
        {

            copy->fillOffset = offset;
            copy->fillCount = path->nfill;

            memcpy(&gl->verts[offset], path->fill, sizeof (struct nvg_vertex) * path->nfill);

            offset += path->nfill;

        }

        if (path->nstroke > 0)
        {

            copy->strokeOffset = offset;
            copy->strokeCount = path->nstroke;

            memcpy(&gl->verts[offset], path->stroke, sizeof (struct nvg_vertex) * path->nstroke);

            offset += path->nstroke;

        }

    }

    if (call->type == GLNVG_FILL)
    {

        call->triangleOffset = offset;
        quad = &gl->verts[call->triangleOffset];

        vset(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
        vset(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
        vset(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
        vset(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);

        call->uniformOffset = allocFragUniforms(gl, 2);

        if (call->uniformOffset == -1)
            goto error;

        frag = fragUniformPtr(gl, call->uniformOffset);

        memset(frag, 0, sizeof (struct nvg_gl_fraguniforms));

        frag->strokeThr = -1.0f;
        frag->type = NSVG_SHADER_SIMPLE;

        convertPaint(gl, fragUniformPtr(gl, call->uniformOffset + gl->fragSize), paint, scissor, fringe, fringe, -1.0f);

    }

    else
    {

        call->uniformOffset = allocFragUniforms(gl, 1);

        if (call->uniformOffset == -1)
            goto error;

        convertPaint(gl, fragUniformPtr(gl, call->uniformOffset), paint, scissor, fringe, fringe, -1.0f);

    }

    return;

error:
    if (gl->ncalls > 0)
        gl->ncalls--;

}

static void renderStroke(void *uptr, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, float fringe, float strokeWidth, const struct nvg_path *paths, int npaths)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    struct nvg_gl_call *call = allocCall(gl);
    int i, maxverts, offset;

    if (call == NULL)
        return;

    call->type = GLNVG_STROKE;
    call->pathOffset = allocPaths(gl, npaths);

    if (call->pathOffset == -1)
        goto error;

    call->pathCount = npaths;
    call->image = paint->image;
    call->blendFunc = blendCompositeOperation(compositeoperation);
    maxverts = maxVertCount(paths, npaths);
    offset = allocVerts(gl, maxverts);

    if (offset == -1)
        goto error;

    for (i = 0; i < npaths; i++)
    {

        struct nvg_gl_path *copy = &gl->paths[call->pathOffset + i];
        const struct nvg_path *path = &paths[i];

        memset(copy, 0, sizeof (struct nvg_gl_path));

        if (path->nstroke)
        {

            copy->strokeOffset = offset;
            copy->strokeCount = path->nstroke;

            memcpy(&gl->verts[offset], path->stroke, sizeof (struct nvg_vertex) * path->nstroke);

            offset += path->nstroke;

        }

    }

    if (gl->flags & NVG_STENCIL_STROKES)
    {

        call->uniformOffset = allocFragUniforms(gl, 2);

        if (call->uniformOffset == -1)
            goto error;

        convertPaint(gl, fragUniformPtr(gl, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);
        convertPaint(gl, fragUniformPtr(gl, call->uniformOffset + gl->fragSize), paint, scissor, strokeWidth, fringe, 1.0f - 0.5f/255.0f);

    }

    else
    {

        call->uniformOffset = allocFragUniforms(gl, 1);

        if (call->uniformOffset == -1)
            goto error;

        convertPaint(gl, fragUniformPtr(gl, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);

    }

    return;

error:
    if (gl->ncalls > 0)
        gl->ncalls--;

}

static void renderTriangles(void *uptr, struct nvg_paint *paint, struct nvg_compositeoperationstate compositeoperation, struct nvg_scissor *scissor, const struct nvg_vertex *verts, int nverts)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    struct nvg_gl_call *call = allocCall(gl);
    struct nvg_gl_fraguniforms *frag;

    if (call == NULL)
        return;

    call->type = GLNVG_TRIANGLES;
    call->image = paint->image;
    call->blendFunc = blendCompositeOperation(compositeoperation);
    call->triangleOffset = allocVerts(gl, nverts);

    if (call->triangleOffset == -1)
        goto error;

    call->triangleCount = nverts;

    memcpy(&gl->verts[call->triangleOffset], verts, sizeof (struct nvg_vertex) * nverts);

    call->uniformOffset = allocFragUniforms(gl, 1);

    if (call->uniformOffset == -1)
        goto error;

    frag = fragUniformPtr(gl, call->uniformOffset);

    convertPaint(gl, frag, paint, scissor, 1.0f, 1.0f, -1.0f);

    frag->type = NSVG_SHADER_IMG;

    return;

error:
    if (gl->ncalls > 0)
        gl->ncalls--;

}

static void renderDelete(void *uptr)
{

    struct nvg_gl_context *gl = (struct nvg_gl_context *)uptr;
    int i;

    if (gl == NULL)
        return;

    deleteShader(&gl->shader);

#if NANOVG_GL3
#if NANOVG_GL_USE_UNIFORMBUFFER
    if (gl->fragBuf != 0)
        glDeleteBuffers(1, &gl->fragBuf);
#endif
    if (gl->vertArr != 0)
        glDeleteVertexArrays(1, &gl->vertArr);
#endif
    if (gl->vertBuf != 0)
        glDeleteBuffers(1, &gl->vertBuf);

    for (i = 0; i < gl->ntextures; i++)
    {

        if (gl->textures[i].tex != 0 && (gl->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
            glDeleteTextures(1, &gl->textures[i].tex);

    }

    free(gl->textures);
    free(gl->paths);
    free(gl->verts);
    free(gl->uniforms);
    free(gl->calls);
    free(gl);

}

struct nvg_context *nvg_create(int flags)
{

    struct nvg_params params;
    struct nvg_context *ctx = NULL;
    struct nvg_gl_context *gl = malloc(sizeof (struct nvg_gl_context));

    if (gl == NULL)
        goto error;

    memset(gl, 0, sizeof (struct nvg_gl_context));
    memset(&params, 0, sizeof (struct nvg_params));

    params.renderCreate = renderCreate;
    params.renderCreateTexture = renderCreateTexture;
    params.renderDeleteTexture = renderDeleteTexture;
    params.renderUpdateTexture = renderUpdateTexture;
    params.renderGetTextureSize = renderGetTextureSize;
    params.renderViewport = renderViewport;
    params.renderCancel = renderCancel;
    params.renderFlush = renderFlush;
    params.renderFill = renderFill;
    params.renderStroke = renderStroke;
    params.renderTriangles = renderTriangles;
    params.renderDelete = renderDelete;
    params.userPtr = gl;
    params.edgeAntiAlias = flags & NVG_ANTIALIAS ? 1 : 0;
    gl->flags = flags;
    ctx = nvgCreateInternal(&params);

    if (ctx == NULL)
        goto error;

    return ctx;

error:
    if (ctx != NULL)
        nvgDeleteInternal(ctx);

    return NULL;

}

void nvg_delete(struct nvg_context *ctx)
{

    nvgDeleteInternal(ctx);

}

