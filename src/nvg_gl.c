#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "nvg.h"
#include "nvg_gl.h"

#define NVG_GL_VERSION_GL2 1
#define NVG_GL_VERSION_GL3 0
#define NVG_GL_VERSION_GLES2 0
#define NVG_GL_VERSION_GLES3 0
#define NVG_GL_UNIFORMARRAYSIZE 11

enum nvg_gl_uniformloc
{

    NVG_GL_LOC_VIEWSIZE,
    NVG_GL_LOC_TEX,
    NVG_GL_LOC_FRAG

};

enum nvg_gl_shadertype
{

    NSVG_SHADER_FILLGRAD,
    NSVG_SHADER_FILLIMG,
    NSVG_SHADER_SIMPLE,
    NSVG_SHADER_IMG

};

enum nvg_gl_calltype
{

    NVG_GL_NONE = 0,
    NVG_GL_FILL,
    NVG_GL_CONVEXFILL,
    NVG_GL_TRIANGLES,

};

struct nvg_gl_fraguniforms
{

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

};

static int maxi(int a, int b)
{

    return a > b ? a : b;

}

struct nvg_gl_texture *nvg_gl_findtexture(struct nvg_gl_context *glctx, int id)
{

    unsigned int i;

    for (i = 0; i < glctx->ntextures; i++)
    {

        if (glctx->textures[i].id == id)
            return &glctx->textures[i];

    }

    return 0;

}

static struct nvg_gl_texture *alloctexture(struct nvg_gl_context *glctx)
{

    struct nvg_gl_texture *tex = nvg_gl_findtexture(glctx, 0);

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

    tex->id = ++glctx->textureid;

    return tex;

}

static void xformtomat3x4(float *m3, float *t)
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

static int convertpaint(struct nvg_gl_context *glctx, struct nvg_gl_fraguniforms *frag, struct nvg_paint *paint, struct nvg_scissor *scissor)
{

    struct nvg_gl_texture *tex = 0;
    float invxform[6];

    memset(frag, 0, sizeof (struct nvg_gl_fraguniforms));

    frag->innerCol = nvg_premulrgba(paint->innerColor);
    frag->outerCol = nvg_premulrgba(paint->outerColor);

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

        nvg_xform_inverse(invxform, scissor->xform);
        xformtomat3x4(frag->scissorMat, invxform);

        frag->scissorExt[0] = scissor->extent[0];
        frag->scissorExt[1] = scissor->extent[1];
        frag->scissorScale[0] = sqrtf(scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]);
        frag->scissorScale[1] = sqrtf(scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]);

    }

    memcpy(frag->extent, paint->extent, sizeof (frag->extent));

    if (paint->image != 0)
    {

        tex = nvg_gl_findtexture(glctx, paint->image);

        if (!tex)
            return 0;

        if ((tex->flags & NVG_IMAGE_FLIPY) != 0)
        {

            float m1[6], m2[6];

            nvg_xform_translate(m1, 0.0f, frag->extent[1] * 0.5f);
            nvg_xform_multiply(m1, paint->xform);
            nvg_xform_scale(m2, 1.0f, -1.0f);
            nvg_xform_multiply(m2, m1);
            nvg_xform_translate(m1, 0.0f, -frag->extent[1] * 0.5f);
            nvg_xform_multiply(m1, m2);
            nvg_xform_inverse(invxform, m1);

        }

        else
        {

            nvg_xform_inverse(invxform, paint->xform);

        }

        frag->type = NSVG_SHADER_FILLIMG;

        if (tex->type == NVG_TEXTURE_RGBA)
            frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
        else
            frag->texType = 2.0f;

    }

    else
    {

        frag->type = NSVG_SHADER_FILLGRAD;
        frag->radius = paint->radius;
        frag->feather = paint->feather;

        nvg_xform_inverse(invxform, paint->xform);

    }

    xformtomat3x4(frag->paintMat, invxform);

    return 1;

}

static struct nvg_gl_fraguniforms *fraguniformptr(struct nvg_gl_context *glctx, int i)
{

    return (struct nvg_gl_fraguniforms *)&glctx->uniforms[i];

}

static void setuniforms(struct nvg_gl_context *glctx, int uniformoffset, int image)
{

    struct nvg_gl_fraguniforms *frag = fraguniformptr(glctx, uniformoffset);

    glUniform4fv(glctx->shader.loc[NVG_GL_LOC_FRAG], NVG_GL_UNIFORMARRAYSIZE, &(frag->uniformArray[0][0]));

    if (image != 0)
    {

        struct nvg_gl_texture *tex = nvg_gl_findtexture(glctx, image);

        glBindTexture(GL_TEXTURE_2D, tex ? tex->tex : 0);

    }

    else
    {

        glBindTexture(GL_TEXTURE_2D, 0);

    }

}

void nvg_gl_flush(struct nvg_gl_context *glctx)
{

    unsigned int i;

    if (!glctx->ncalls)
        return;

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
#if defined NVG_GL_VERSION_GL3
    glBindVertexArray(glctx->vertArr);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, glctx->vertBuf);
    glBufferData(GL_ARRAY_BUFFER, glctx->nverts * sizeof (struct nvg_vertex), glctx->verts, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (struct nvg_vertex), (const GLvoid *)(size_t)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (struct nvg_vertex), (const GLvoid *)(2 * sizeof (float)));
    glUniform1i(glctx->shader.loc[NVG_GL_LOC_TEX], 0);
    glUniform2fv(glctx->shader.loc[NVG_GL_LOC_VIEWSIZE], 1, glctx->view);

    for (i = 0; i < glctx->ncalls; i++)
    {

        struct nvg_gl_call *call = &glctx->calls[i];

        glBlendFuncSeparate(call->blendfunc.srcrgb, call->blendfunc.dstrgb, call->blendfunc.srcalpha, call->blendfunc.dstalpha);

        if (call->type == NVG_GL_FILL)
        {

            struct nvg_gl_path *paths = &glctx->paths[call->pathoffset];
            int j;

            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_ALWAYS, 0, 0xff);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            setuniforms(glctx, call->uniformoffset, 0);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
            glDisable(GL_CULL_FACE);

            for (j = 0; j < call->pathcount; j++)
                glDrawArrays(GL_TRIANGLE_FAN, paths[j].filloffset, paths[j].fillcount);

            glEnable(GL_CULL_FACE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            setuniforms(glctx, call->uniformoffset + glctx->fragsize, call->image);
            glStencilFunc(GL_NOTEQUAL, 0x0, 0xff);
            glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
            glDrawArrays(GL_TRIANGLE_STRIP, call->triangleoffset, call->trianglecount);
            glDisable(GL_STENCIL_TEST);

        }

        else if (call->type == NVG_GL_CONVEXFILL)
        {

            struct nvg_gl_path *paths = &glctx->paths[call->pathoffset];
            int j;

            setuniforms(glctx, call->uniformoffset, call->image);

            for (j = 0; j < call->pathcount; j++)
                glDrawArrays(GL_TRIANGLE_FAN, paths[j].filloffset, paths[j].fillcount);

        }

        else if (call->type == NVG_GL_TRIANGLES)
        {

            setuniforms(glctx, call->uniformoffset, call->image);
            glDrawArrays(GL_TRIANGLES, call->triangleoffset, call->trianglecount);

        }

    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
#if defined NVG_GL_VERSION_GL3
    glBindVertexArray(0);
#endif
    glDisable(GL_CULL_FACE);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

static int maxvertcount(const struct nvg_path *paths, int npaths)
{

    int count = 0;
    int i;

    for (i = 0; i < npaths; i++)
        count += paths[i].nfill;

    return count;

}

static struct nvg_gl_call *alloccall(struct nvg_gl_context *glctx)
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

static int allocpaths(struct nvg_gl_context *glctx, int n)
{

    int ret;

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

static int allocverts(struct nvg_gl_context *glctx, int n)
{

    int ret;

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

static int allocfraguniforms(struct nvg_gl_context *glctx, int n)
{

    int ret;

    if (glctx->nuniforms + n > glctx->cuniforms)
    {

        int cuniforms = maxi(glctx->nuniforms + n, 128) + glctx->cuniforms / 2;
        unsigned char *uniforms = realloc(glctx->uniforms, glctx->fragsize * cuniforms);

        if (!uniforms)
            return -1;

        glctx->uniforms = uniforms;
        glctx->cuniforms = cuniforms;

    }

    ret = glctx->nuniforms * glctx->fragsize;
    glctx->nuniforms += n;

    return ret;

}

void nvg_gl_render_paths(struct nvg_gl_context *glctx, struct nvg_paint *paint, struct nvg_scissor *scissor, const float *bounds, const struct nvg_path *paths, int npaths)
{

    struct nvg_gl_call *call = alloccall(glctx);
    struct nvg_vertex *quad;
    struct nvg_gl_fraguniforms *frag;
    int i, maxverts, offset;

    if (!call)
        return;

    call->type = NVG_GL_FILL;
    call->trianglecount = 4;
    call->pathoffset = allocpaths(glctx, npaths);

    if (call->pathoffset == -1)
        goto error;

    call->pathcount = npaths;
    call->image = paint->image;
    call->blendfunc = glctx->blendfunc;

    if (npaths == 1 && paths[0].convex)
    {

        call->type = NVG_GL_CONVEXFILL;
        call->trianglecount = 0;

    }

    maxverts = maxvertcount(paths, npaths) + call->trianglecount;
    offset = allocverts(glctx, maxverts);

    if (offset == -1)
        goto error;

    for (i = 0; i < npaths; i++)
    {

        struct nvg_gl_path *copy = &glctx->paths[call->pathoffset + i];
        const struct nvg_path *path = &paths[i];

        memset(copy, 0, sizeof (struct nvg_gl_path));

        if (path->nfill > 0)
        {

            copy->filloffset = offset;
            copy->fillcount = path->nfill;

            memcpy(&glctx->verts[offset], path->fill, sizeof (struct nvg_vertex) * path->nfill);

            offset += path->nfill;

        }

    }

    if (call->type == NVG_GL_FILL)
    {

        call->triangleoffset = offset;
        quad = &glctx->verts[call->triangleoffset];

        nvg_setvertex(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
        nvg_setvertex(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
        nvg_setvertex(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
        nvg_setvertex(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);

        call->uniformoffset = allocfraguniforms(glctx, 2);

        if (call->uniformoffset == -1)
            goto error;

        frag = fraguniformptr(glctx, call->uniformoffset);

        memset(frag, 0, sizeof (struct nvg_gl_fraguniforms));

        frag->type = NSVG_SHADER_SIMPLE;

        convertpaint(glctx, fraguniformptr(glctx, call->uniformoffset + glctx->fragsize), paint, scissor);

    }

    else
    {

        call->uniformoffset = allocfraguniforms(glctx, 1);

        if (call->uniformoffset == -1)
            goto error;

        convertpaint(glctx, fraguniformptr(glctx, call->uniformoffset), paint, scissor);

    }

    return;

error:
    if (glctx->ncalls > 0)
        glctx->ncalls--;

}

void nvg_gl_render_vertices(struct nvg_gl_context *glctx, struct nvg_paint *paint, struct nvg_scissor *scissor, const struct nvg_vertex *verts, int nverts)
{

    struct nvg_gl_call *call = alloccall(glctx);
    struct nvg_gl_fraguniforms *frag;

    if (!call)
        return;

    call->type = NVG_GL_TRIANGLES;
    call->image = paint->image;
    call->blendfunc = glctx->blendfunc;
    call->triangleoffset = allocverts(glctx, nverts);

    if (call->triangleoffset == -1)
        goto error;

    call->trianglecount = nverts;

    memcpy(&glctx->verts[call->triangleoffset], verts, sizeof (struct nvg_vertex) * nverts);

    call->uniformoffset = allocfraguniforms(glctx, 1);

    if (call->uniformoffset == -1)
        goto error;

    frag = fraguniformptr(glctx, call->uniformoffset);

    convertpaint(glctx, frag, paint, scissor);

    frag->type = NSVG_SHADER_IMG;

    return;

error:
    if (glctx->ncalls > 0)
        glctx->ncalls--;

}

void nvg_gl_reset(struct nvg_gl_context *glctx, float width, float height)
{

    glctx->view[0] = width;
    glctx->view[1] = height;
    glctx->blendfunc.srcrgb = GL_ONE;
    glctx->blendfunc.dstrgb = GL_ONE_MINUS_SRC_ALPHA;
    glctx->blendfunc.srcalpha = GL_ONE;
    glctx->blendfunc.dstalpha = GL_ONE_MINUS_SRC_ALPHA;
    glctx->nverts = 0;
    glctx->npaths = 0;
    glctx->ncalls = 0;
    glctx->nuniforms = 0;

}

int nvg_gl_texture_update(struct nvg_gl_context *glctx, int image, int x, int y, int w, int h, const unsigned char *data)
{

    struct nvg_gl_texture *texture = nvg_gl_findtexture(glctx, image);

    if (!texture)
        return 0;

    glBindTexture(GL_TEXTURE_2D, texture->tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifndef NVG_GL_VERSION_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
#else
    if (texture->type == NVG_TEXTURE_RGBA)
        data += y * texture->width * 4;
    else
        data += y * texture->width;

    x = 0;
    w = texture->width;
#endif

    if (texture->type == NVG_TEXTURE_RGBA)
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
    glBindTexture(GL_TEXTURE_2D, 0);

    return 1;

}

int nvg_gl_texture_create(struct nvg_gl_context *glctx, int type, int w, int h, int flags, const unsigned char *data)
{

    struct nvg_gl_texture *texture = alloctexture(glctx);

    if (!texture)
        return 0;

    glGenTextures(1, &texture->tex);

    texture->width = w;
    texture->height = h;
    texture->type = type;
    texture->flags = flags;

    glBindTexture(GL_TEXTURE_2D, texture->tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifndef NVG_GL_VERSION_GLES2
    glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

#if defined (NVG_GL_VERSION_GL2)
    if (flags & NVG_IMAGE_GENERATE_MIPMAPS)
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

    if (flags & NVG_IMAGE_GENERATE_MIPMAPS)
    {

        if (flags & NVG_IMAGE_NEAREST)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    }

    else
    {

        if (flags & NVG_IMAGE_NEAREST)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    }

    if (flags & NVG_IMAGE_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (flags & NVG_IMAGE_REPEATX)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    if (flags & NVG_IMAGE_REPEATY)
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
    if (flags & NVG_IMAGE_GENERATE_MIPMAPS)
        glGenerateMipmap(GL_TEXTURE_2D);
#endif

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture->id;

}

void nvg_gl_texture_destroy(struct nvg_gl_context *glctx, int id)
{

    struct nvg_gl_texture *texture = nvg_gl_findtexture(glctx, id);

    if (texture)
    {

        if (texture->tex != 0)
            glDeleteTextures(1, &texture->tex);

        memset(texture, 0, sizeof (struct nvg_gl_texture));

    }

}

void nvg_gl_create(struct nvg_gl_context *glctx, int w, int h)
{

    GLuint prog, vert, frag;
    const char *str[2];

    static const char *header =
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
    "#define UNIFORMARRAYSIZE 11\n"
    "\n";

    static const char *vertexfill =
        "uniform vec2 viewsize;\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "    in vec2 vertex;\n"
        "    in vec2 tcoord;\n"
        "    out vec2 ftcoord;\n"
        "    out vec2 fpos;\n"
        "#else\n"
        "    attribute vec2 vertex;\n"
        "    attribute vec2 tcoord;\n"
        "    varying vec2 ftcoord;\n"
        "    varying vec2 fpos;\n"
        "#endif\n"
        "void main(void) {\n"
        "    ftcoord = tcoord;\n"
        "    fpos = vertex;\n"
        "    gl_Position = vec4(2.0 * vertex.x / viewsize.x - 1.0, 1.0 - 2.0 * vertex.y / viewsize.y, 0, 1);\n"
        "}\n";

    static const char *fragmentfill =
        "#ifdef GL_ES\n"
        "#if defined(GL_FRAGMENT_PRECISION_HIGH) || defined(NVG_GL_VERSION_GL3)\n"
        "    precision highp float;\n"
        "#else\n"
        "    precision mediump float;\n"
        "#endif\n"
        "#endif\n"
        "uniform vec4 frag[UNIFORMARRAYSIZE];\n"
        "uniform sampler2D tex;\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "    in vec2 ftcoord;\n"
        "    in vec2 fpos;\n"
        "    out vec4 outColor;\n"
        "#else\n"
        "    varying vec2 ftcoord;\n"
        "    varying vec2 fpos;\n"
        "#endif\n"
        "#define scissorMat mat3(frag[0].xyz, frag[1].xyz, frag[2].xyz)\n"
        "#define paintMat mat3(frag[3].xyz, frag[4].xyz, frag[5].xyz)\n"
        "#define innerCol frag[6]\n"
        "#define outerCol frag[7]\n"
        "#define scissorExt frag[8].xy\n"
        "#define scissorScale frag[8].zw\n"
        "#define extent frag[9].xy\n"
        "#define radius frag[9].z\n"
        "#define feather frag[9].w\n"
        "#define texType int(frag[10].z)\n"
        "#define type int(frag[10].w)\n"
        "float sdroundrect(vec2 pt, vec2 ext, float rad) {\n"
        "    vec2 ext2 = ext - vec2(rad, rad);\n"
        "    vec2 d = abs(pt) - ext2;\n"
        "    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;\n"
        "}\n"
        "float scissorMask(vec2 p) {\n"
        "    vec2 sc = (abs((scissorMat * vec3(p, 1.0)).xy) - scissorExt);\n"
        "    sc = vec2(0.5, 0.5) - sc * scissorScale;\n"
        "    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);\n"
        "}\n"
        "void main(void) {\n"
        "    vec4 result;\n"
        "    float scissor = scissorMask(fpos);\n"
        "    float strokeAlpha = 1.0;\n"
        "    if (type == 0) {\n"
        "        vec2 pt = (paintMat * vec3(fpos, 1.0)).xy;\n"
        "        float d = clamp((sdroundrect(pt, extent, radius) + feather * 0.5) / feather, 0.0, 1.0);\n"
        "        vec4 color = mix(innerCol,outerCol, d);\n"
        "        color *= strokeAlpha * scissor;\n"
        "        result = color;\n"
        "    } else if (type == 1) {\n"
        "        vec2 pt = (paintMat * vec3(fpos, 1.0)).xy / extent;\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "        vec4 color = texture(tex, pt);\n"
        "#else\n"
        "        vec4 color = texture2D(tex, pt);\n"
        "#endif\n"
        "        if (texType == 1) color = vec4(color.xyz * color.w, color.w);"
        "        if (texType == 2) color = vec4(color.x);"
        "        color *= innerCol;\n"
        "        color *= strokeAlpha * scissor;\n"
        "        result = color;\n"
        "    } else if (type == 2) {\n"
        "        result = vec4(1, 1, 1, 1);\n"
        "    } else if (type == 3) {\n"
        "#ifdef NVG_GL_VERSION_GL3\n"
        "        vec4 color = texture(tex, ftcoord);\n"
        "#else\n"
        "        vec4 color = texture2D(tex, ftcoord);\n"
        "#endif\n"
        "        if (texType == 1) color = vec4(color.xyz * color.w, color.w);"
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

    str[0] = header;

    prog = glCreateProgram();
    vert = glCreateShader(GL_VERTEX_SHADER);
    frag = glCreateShader(GL_FRAGMENT_SHADER);

    str[1] = vertexfill;

    glShaderSource(vert, 2, str, 0);

    str[1] = fragmentfill;

    glShaderSource(frag, 2, str, 0);
    glCompileShader(vert);
    glCompileShader(frag);
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glBindAttribLocation(prog, 0, "vertex");
    glBindAttribLocation(prog, 1, "tcoord");
    glLinkProgram(prog);

    glctx->shader.prog = prog;
    glctx->shader.vert = vert;
    glctx->shader.frag = frag;
    glctx->shader.loc[NVG_GL_LOC_VIEWSIZE] = glGetUniformLocation(glctx->shader.prog, "viewsize");
    glctx->shader.loc[NVG_GL_LOC_TEX] = glGetUniformLocation(glctx->shader.prog, "tex");
    glctx->shader.loc[NVG_GL_LOC_FRAG] = glGetUniformLocation(glctx->shader.prog, "frag");

#if defined NVG_GL_VERSION_GL3
    glGenVertexArrays(1, &glctx->vertArr);
#endif
    glGenBuffers(1, &glctx->vertBuf);
    glFinish();

    glctx->fragsize = sizeof (struct nvg_gl_fraguniforms) + 4 - sizeof (struct nvg_gl_fraguniforms) % 4;
    glctx->fontimage = nvg_gl_texture_create(glctx, NVG_TEXTURE_ALPHA, w, h, 0, 0);

}

void nvg_gl_destroy(struct nvg_gl_context *glctx)
{

    int i;

    nvg_gl_texture_destroy(glctx, glctx->fontimage);

    if (glctx->shader.prog)
        glDeleteProgram(glctx->shader.prog);

    if (glctx->shader.vert)
        glDeleteShader(glctx->shader.vert);

    if (glctx->shader.frag)
        glDeleteShader(glctx->shader.frag);

#if NVG_GL_VERSION_GL3
    if (glctx->vertArr != 0)
        glDeleteVertexArrays(1, &glctx->vertArr);
#endif

    if (glctx->vertBuf != 0)
        glDeleteBuffers(1, &glctx->vertBuf);

    for (i = 0; i < glctx->ntextures; i++)
    {

        if (glctx->textures[i].tex != 0)
            glDeleteTextures(1, &glctx->textures[i].tex);

    }

    free(glctx->textures);
    free(glctx->paths);
    free(glctx->verts);
    free(glctx->uniforms);
    free(glctx->calls);

}

