#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "fons.h"

static int tt_loadFont(struct fons_context *context, struct fons_impl *font, unsigned char *data, int dataSize)
{

    font->font.userdata = context;

    return stbtt_InitFont(&font->font, data, 0);

}

static void tt_getFontVMetrics(struct fons_impl *font, int *ascent, int *descent, int *lineGap)
{

    stbtt_GetFontVMetrics(&font->font, ascent, descent, lineGap);

}

static float tt_getPixelHeightScale(struct fons_impl *font, float size)
{

    return stbtt_ScaleForPixelHeight(&font->font, size);

}

static int tt_getGlyphIndex(struct fons_impl *font, int codepoint)
{

    return stbtt_FindGlyphIndex(&font->font, codepoint);

}

static int tt_buildGlyphBitmap(struct fons_impl *font, int glyph, float size, float scale, int *advance, int *lsb, int *x0, int *y0, int *x1, int *y1)
{

    stbtt_GetGlyphHMetrics(&font->font, glyph, advance, lsb);
    stbtt_GetGlyphBitmapBox(&font->font, glyph, scale, scale, x0, y0, x1, y1);

    return 1;

}

static void tt_renderGlyphBitmap(struct fons_impl *font, unsigned char *output, int outWidth, int outHeight, int outStride, float scaleX, float scaleY, int glyph)
{

    stbtt_MakeGlyphBitmap(&font->font, output, outWidth, outHeight, outStride, scaleX, scaleY, glyph);

}

static int tt_getGlyphKernAdvance(struct fons_impl *font, int glyph1, int glyph2)
{

    return stbtt_GetGlyphKernAdvance(&font->font, glyph1, glyph2);

}

static unsigned int hashint(unsigned int a)
{

    a += ~(a << 15);
    a ^=  (a >> 10);
    a +=  (a << 3);
    a ^=  (a >> 6);
    a += ~(a << 11);
    a ^=  (a >> 16);

    return a;

}

static int mini(int a, int b)
{

    return a < b ? a : b;

}

static int maxi(int a, int b)
{

    return a > b ? a : b;

}

static unsigned int decutf8(unsigned int *state, unsigned int *codep, unsigned int byte)
{

    static const unsigned char utf8d[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        0, 12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
        12, 0, 12, 12, 12, 12, 12, 0, 12, 0, 12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 24, 12, 12,
        12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12,
        12, 12, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12,
        12, 36, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
    };

    unsigned int type = utf8d[byte];

    *codep = (*state) ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & (byte);
    *state = utf8d[256 + *state + type];

    return *state;

}

static int atlasInsertNode(struct fons_atlas *atlas, int idx, int x, int y, int w)
{

    int i;

    for (i = atlas->nnodes; i > idx; i--)
        atlas->nodes[i] = atlas->nodes[i - 1];

    atlas->nodes[idx].x = (short)x;
    atlas->nodes[idx].y = (short)y;
    atlas->nodes[idx].width = (short)w;
    atlas->nnodes++;

    return 1;

}

static void atlasRemoveNode(struct fons_atlas *atlas, int idx)
{

    int i;

    if (!atlas->nnodes)
        return;

    for (i = idx; i < atlas->nnodes - 1; i++)
        atlas->nodes[i] = atlas->nodes[i + 1];

    atlas->nnodes--;

}

static int atlasAddSkylineLevel(struct fons_atlas *atlas, int idx, int x, int y, int w, int h)
{

    int i;

    if (!atlasInsertNode(atlas, idx, x, y + h, w))
        return 0;

    for (i = idx + 1; i < atlas->nnodes; i++)
    {

        if (atlas->nodes[i].x < atlas->nodes[i - 1].x + atlas->nodes[i - 1].width)
        {

            int shrink = atlas->nodes[i - 1].x + atlas->nodes[i - 1].width - atlas->nodes[i].x;

            atlas->nodes[i].x += shrink;
            atlas->nodes[i].width -= shrink;
            
            if (atlas->nodes[i].width <= 0)
            {

                atlasRemoveNode(atlas, i);

                i--;

            }

            else
            {

                break;

            }

        }

        else
        {

            break;

        }

    }

    for (i = 0; i < atlas->nnodes - 1; i++)
    {

        if (atlas->nodes[i].y == atlas->nodes[i + 1].y)
        {

            atlas->nodes[i].width += atlas->nodes[i + 1].width;

            atlasRemoveNode(atlas, i + 1);

            i--;

        }

    }

    return 1;

}

static int atlasRectFits(struct fons_atlas *atlas, int i, int w, int h)
{

    int x = atlas->nodes[i].x;
    int y = atlas->nodes[i].y;
    int spaceLeft;

    if (x + w > atlas->width)
        return -1;

    spaceLeft = w;

    while (spaceLeft > 0)
    {

        if (i == atlas->nnodes)
            return -1;

        y = maxi(y, atlas->nodes[i].y);

        if (y + h > atlas->height)
            return -1;

        spaceLeft -= atlas->nodes[i].width;
        ++i;

    }

    return y;

}

static int atlasAddRect(struct fons_atlas *atlas, int rw, int rh, int *rx, int *ry)
{

    int besth = atlas->height;
    int bestw = atlas->width;
    int besti = -1;
    int bestx = -1;
    int besty = -1;
    int i;

    for (i = 0; i < atlas->nnodes; i++)
    {

        int y = atlasRectFits(atlas, i, rw, rh);

        if (y != -1)
        {

            if (y + rh < besth || (y + rh == besth && atlas->nodes[i].width < bestw))
            {

                besti = i;
                bestw = atlas->nodes[i].width;
                besth = y + rh;
                bestx = atlas->nodes[i].x;
                besty = y;

            }

        }

    }

    if (besti == -1)
        return 0;

    if (!atlasAddSkylineLevel(atlas, besti, bestx, besty, rw, rh))
        return 0;

    *rx = bestx;
    *ry = besty;

    return 1;

}

static void addWhiteRect(struct fons_context *fsctx, int w, int h)
{

    int x, y, gx, gy;
    unsigned char *dst;

    if (!atlasAddRect(&fsctx->atlas, w, h, &gx, &gy))
        return;

    dst = &fsctx->texData[gx + gy * fsctx->width];

    for (y = 0; y < h; y++)
    {

        for (x = 0; x < w; x++)
            dst[x] = 0xff;

        dst += fsctx->width;

    }

    fsctx->dirtyRect[0] = mini(fsctx->dirtyRect[0], gx);
    fsctx->dirtyRect[1] = mini(fsctx->dirtyRect[1], gy);
    fsctx->dirtyRect[2] = maxi(fsctx->dirtyRect[2], gx + w);
    fsctx->dirtyRect[3] = maxi(fsctx->dirtyRect[3], gy + h);

}

void fons_create(struct fons_context *fsctx, int width, int height, unsigned char flags)
{

    fsctx->width = width;
    fsctx->height = height;
    fsctx->flags = flags;
    fsctx->atlas.width = width;
    fsctx->atlas.height = height;
    fsctx->atlas.nodes[0].x = 0;
    fsctx->atlas.nodes[0].y = 0;
    fsctx->atlas.nodes[0].width = width;
    fsctx->atlas.nnodes = 1;
    fsctx->fonts = malloc(sizeof(struct fons_font *) * FONS_INIT_FONTS);

    if (!fsctx->fonts)
        return;

    memset(fsctx->fonts, 0, sizeof(struct fons_font *) * FONS_INIT_FONTS);

    fsctx->cfonts = FONS_INIT_FONTS;
    fsctx->nfonts = 0;
    fsctx->itw = 1.0f / fsctx->width;
    fsctx->ith = 1.0f / fsctx->height;
    fsctx->texData = malloc(fsctx->width * fsctx->height);

    if (!fsctx->texData)
        return;

    memset(fsctx->texData, 0, fsctx->width * fsctx->height);

    fsctx->dirtyRect[0] = fsctx->width;
    fsctx->dirtyRect[1] = fsctx->height;
    fsctx->dirtyRect[2] = 0;
    fsctx->dirtyRect[3] = 0;

    addWhiteRect(fsctx, 2, 2);
    fonsClearState(fsctx);

}

void fonsSetSize(struct fons_context *fsctx, float size)
{

    fsctx->state.size = size;

}

void fonsSetColor(struct fons_context *fsctx, unsigned int color)
{

    fsctx->state.color = color;

}

void fonsSetSpacing(struct fons_context *fsctx, float spacing)
{

    fsctx->state.spacing = spacing;

}

void fonsSetBlur(struct fons_context *fsctx, float blur)
{

    fsctx->state.blur = blur;

}

void fonsSetAlign(struct fons_context *fsctx, int align)
{

    fsctx->state.align = align;

}

void fonsSetFont(struct fons_context *fsctx, int font)
{

    fsctx->state.font = font;

}

void fonsClearState(struct fons_context *fsctx)
{

    fsctx->state.size = 12.0f;
    fsctx->state.color = 0xffffffff;
    fsctx->state.font = 0;
    fsctx->state.blur = 0;
    fsctx->state.spacing = 0;
    fsctx->state.align = FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE;

}

static void freeFont(struct fons_font *font)
{

    if (font->data)
        free(font->data);

    free(font);

}

static int allocFont(struct fons_context *fsctx)
{

    struct fons_font *font = 0;

    if (fsctx->nfonts + 1 > fsctx->cfonts)
    {

        fsctx->cfonts = fsctx->cfonts == 0 ? 8 : fsctx->cfonts * 2;
        fsctx->fonts = realloc(fsctx->fonts, sizeof(struct fons_font*) * fsctx->cfonts);

        if (!fsctx->fonts)
            return -1;

    }

    font = malloc(sizeof(struct fons_font));

    if (!font)
        goto error;

    memset(font, 0, sizeof(struct fons_font));

    font->nglyphs = 0;
    fsctx->fonts[fsctx->nfonts++] = font;

    return fsctx->nfonts - 1;

error:
    freeFont(font);

    return -1;

}

int fonsAddFont(struct fons_context *fsctx, const char *path)
{

    FILE *fp = 0;
    int dataSize = 0;
    size_t readed;
    unsigned char *data = 0;

    fp = fopen(path, "rb");

    if (!fp)
        goto error;

    fseek(fp, 0, SEEK_END);

    dataSize = (int)ftell(fp);

    fseek(fp, 0, SEEK_SET);

    data = malloc(dataSize);

    if (!data)
        goto error;

    readed = fread(data, 1, dataSize, fp);

    fclose(fp);

    fp = 0;

    if (readed != dataSize)
        goto error;

    return fonsAddFontMem(fsctx, data, dataSize);

error:
    if (data)
        free(data);

    if (fp)
        fclose(fp);

    return -1;

}

int fonsAddFontMem(struct fons_context *fsctx, unsigned char *data, int dataSize)
{

    int i, ascent, descent, fh, lineGap;
    struct fons_font *font;
    int idx = allocFont(fsctx);

    if (idx == -1)
        return -1;

    font = fsctx->fonts[idx];

    for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
        font->lut[i] = -1;

    font->dataSize = dataSize;
    font->data = data;

    if (!tt_loadFont(fsctx, &font->font, data, dataSize))
        goto error;

    tt_getFontVMetrics( &font->font, &ascent, &descent, &lineGap);

    fh = ascent - descent;
    font->ascender = (float)ascent / (float)fh;
    font->descender = (float)descent / (float)fh;
    font->lineh = (float)(fh + lineGap) / (float)fh;

    return idx;

error:
    freeFont(font);

    fsctx->nfonts--;

    return -1;

}

static struct fons_glyph *allocGlyph(struct fons_font *font)
{

    font->nglyphs++;

    return &font->glyphs[font->nglyphs - 1];

}

#define APREC 16
#define ZPREC 7

static void blurCols(unsigned char *dst, int w, int h, int dstStride, int alpha)
{

    int x, y;

    for (y = 0; y < h; y++)
    {

        int z = 0;

        for (x = 1; x < w; x++)
        {

            z += (alpha * (((int)(dst[x]) << ZPREC) - z)) >> APREC;
            dst[x] = z >> ZPREC;

        }

        dst[w - 1] = 0;
        z = 0;

        for (x = w - 2; x >= 0; x--)
        {

            z += (alpha * (((int)(dst[x]) << ZPREC) - z)) >> APREC;
            dst[x] = z >> ZPREC;

        }

        dst[0] = 0;
        dst += dstStride;

    }

}

static void blurRows(unsigned char *dst, int w, int h, int dstStride, int alpha)
{

    int x, y;

    for (x = 0; x < w; x++)
    {

        int z = 0;

        for (y = dstStride; y < h * dstStride; y += dstStride)
        {

            z += (alpha * (((int)(dst[y]) << ZPREC) - z)) >> APREC;
            dst[y] = z >> ZPREC;

        }

        dst[(h - 1) * dstStride] = 0;
        z = 0;

        for (y = (h - 2) * dstStride; y >= 0; y -= dstStride)
        {

            z += (alpha * (((int)(dst[y]) << ZPREC) - z)) >> APREC;
            dst[y] = z >> ZPREC;

        }

        dst[0] = 0;
        dst++;

    }

}

static void blurit(unsigned char *dst, int w, int h, int dstStride, int blur)
{

    int alpha;
    float sigma;

    if (blur < 1)
        return;

    sigma = (float)blur * 0.57735f;
    alpha = (int)((1 << APREC) * (1.0f - expf(-2.3f / (sigma + 1.0f))));
    blurRows(dst, w, h, dstStride, alpha);
    blurCols(dst, w, h, dstStride, alpha);
    blurRows(dst, w, h, dstStride, alpha);
    blurCols(dst, w, h, dstStride, alpha);
//    blurrows(dst, w, h, dstStride, alpha);
//    blurcols(dst, w, h, dstStride, alpha);

}

static struct fons_glyph *getGlyph(struct fons_context *fsctx, struct fons_font *font, unsigned int codepoint, short size, short blur, int bitmapOption)
{

    int i, g, advance, lsb;
    int x0, y0, x1, y1;
    int gw, gh, gx, gy;
    int x, y;
    float scale;
    struct fons_glyph *glyph = 0;
    unsigned int h;
    int pad, added;
    unsigned char *bdst;
    unsigned char *dst;
    struct fons_font *renderFont = font;

    if (size < 2)
        return 0;

    if (blur > 20)
        blur = 20;

    pad = blur + 2;
    h = hashint(codepoint) & (FONS_HASH_LUT_SIZE - 1);
    i = font->lut[h];

    while (i != -1)
    {
    
        if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == size && font->glyphs[i].blur == blur)
        {

            glyph = &font->glyphs[i];

            if (bitmapOption == FONS_GLYPH_BITMAP_OPTIONAL || (glyph->x0 >= 0 && glyph->y0 >= 0))
                return glyph;

            break;

        }

        i = font->glyphs[i].next;

    }

    g = tt_getGlyphIndex(&font->font, codepoint);

    if (!g)
        return 0;

    scale = tt_getPixelHeightScale(&renderFont->font, size);

    tt_buildGlyphBitmap(&renderFont->font, g, size, scale, &advance, &lsb, &x0, &y0, &x1, &y1);

    gw = x1 - x0 + pad * 2;
    gh = y1 - y0 + pad * 2;

    if (bitmapOption == FONS_GLYPH_BITMAP_REQUIRED)
    {

        added = atlasAddRect(&fsctx->atlas, gw, gh, &gx, &gy);

        if (!added)
            return 0;

    }

    else
    {

        gx = -1;
        gy = -1;

    }

    if (!glyph)
    {

        glyph = allocGlyph(font);
        glyph->codepoint = codepoint;
        glyph->size = size;
        glyph->blur = blur;
        glyph->next = 0;
        glyph->next = font->lut[h];
        font->lut[h] = font->nglyphs - 1;

    }

    glyph->index = g;
    glyph->x0 = gx;
    glyph->y0 = gy;
    glyph->x1 = glyph->x0 + gw;
    glyph->y1 = glyph->y0 + gh;
    glyph->xadv = scale * advance;
    glyph->xoff = x0 - pad;
    glyph->yoff = y0 - pad;

    if (bitmapOption == FONS_GLYPH_BITMAP_OPTIONAL)
        return glyph;

    dst = &fsctx->texData[(glyph->x0 + pad) + (glyph->y0 + pad) * fsctx->width];

    tt_renderGlyphBitmap(&renderFont->font, dst, gw - pad * 2, gh - pad * 2, fsctx->width, scale, scale, g);

    dst = &fsctx->texData[glyph->x0 + glyph->y0 * fsctx->width];

    for (y = 0; y < gh; y++)
    {

        dst[y * fsctx->width] = 0;
        dst[gw - 1 + y * fsctx->width] = 0;

    }

    for (x = 0; x < gw; x++)
    {

        dst[x] = 0;
        dst[x + (gh - 1) * fsctx->width] = 0;

    }

    if (blur > 0)
    {

        bdst = &fsctx->texData[glyph->x0 + glyph->y0 * fsctx->width];

        blurit(bdst, gw, gh, fsctx->width, blur);

    }

    fsctx->dirtyRect[0] = mini(fsctx->dirtyRect[0], glyph->x0);
    fsctx->dirtyRect[1] = mini(fsctx->dirtyRect[1], glyph->y0);
    fsctx->dirtyRect[2] = maxi(fsctx->dirtyRect[2], glyph->x1);
    fsctx->dirtyRect[3] = maxi(fsctx->dirtyRect[3], glyph->y1);

    return glyph;

}

static void getQuad(struct fons_context *fsctx, struct fons_font *font, int prevGlyphIndex, struct fons_glyph *glyph, float scale, float spacing, float *x, float *y, struct fons_quad *q)
{

    float rx, ry;
    float xoff, yoff;
    float x0, y0, x1, y1;

    if (prevGlyphIndex != -1)
    {

        float adv = tt_getGlyphKernAdvance(&font->font, prevGlyphIndex, glyph->index) * scale;

        *x += (int)(adv + spacing + 0.5f);

    }

    xoff = glyph->xoff + 1;
    yoff = glyph->yoff + 1;
    x0 = glyph->x0 + 1;
    y0 = glyph->y0 + 1;
    x1 = glyph->x1 - 1;
    y1 = glyph->y1 - 1;

    if (fsctx->flags & FONS_ZERO_TOPLEFT)
    {

        rx = (float)(int)(*x + xoff);
        ry = (float)(int)(*y + yoff);
        q->x0 = rx;
        q->y0 = ry;
        q->x1 = rx + x1 - x0;
        q->y1 = ry + y1 - y0;
        q->s0 = x0 * fsctx->itw;
        q->t0 = y0 * fsctx->ith;
        q->s1 = x1 * fsctx->itw;
        q->t1 = y1 * fsctx->ith;

    }

    else
    {

        rx = (float)(int)(*x + xoff);
        ry = (float)(int)(*y - yoff);
        q->x0 = rx;
        q->y0 = ry;
        q->x1 = rx + x1 - x0;
        q->y1 = ry - y1 + y0;
        q->s0 = x0 * fsctx->itw;
        q->t0 = y0 * fsctx->ith;
        q->s1 = x1 * fsctx->itw;
        q->t1 = y1 * fsctx->ith;

    }

    *x += (int)(glyph->xadv);

}

static float getVertAlign(struct fons_context *fsctx, struct fons_font *font, int align, short size)
{

    if (fsctx->flags & FONS_ZERO_TOPLEFT)
    {

        if (align & FONS_ALIGN_TOP)
            return font->ascender * size;
        else if (align & FONS_ALIGN_MIDDLE)
            return (font->ascender + font->descender) / 2.0f * size;
        else if (align & FONS_ALIGN_BASELINE)
            return 0.0f;
        else if (align & FONS_ALIGN_BOTTOM)
            return font->descender * size;

    }

    else
    {

        if (align & FONS_ALIGN_TOP)
            return -font->ascender * size;
        else if (align & FONS_ALIGN_MIDDLE)
            return -(font->ascender + font->descender) / 2.0f * size;
        else if (align & FONS_ALIGN_BASELINE)
            return 0.0f;
        else if (align & FONS_ALIGN_BOTTOM)
            return -font->descender * size;

    }

    return 0.0;

}

int fonsTextIterInit(struct fons_context *fsctx, struct fons_textiter *iter, float x, float y, const char *str, const char *end, int bitmapOption)
{

    float width;

    memset(iter, 0, sizeof(*iter));

    iter->font = fsctx->fonts[fsctx->state.font];
    iter->size = fsctx->state.size;
    iter->blur = fsctx->state.blur;
    iter->scale = tt_getPixelHeightScale(&iter->font->font, iter->size);

    if (fsctx->state.align & FONS_ALIGN_LEFT)
    {

    }

    else if (fsctx->state.align & FONS_ALIGN_RIGHT)
    {

        width = fonsTextBounds(fsctx, x,y, str, end, 0);
        x -= width;

    }

    else if (fsctx->state.align & FONS_ALIGN_CENTER)
    {

        width = fonsTextBounds(fsctx, x,y, str, end, 0);
        x -= width * 0.5f;

    }

    y += getVertAlign(fsctx, iter->font, fsctx->state.align, iter->size);
    iter->x = iter->nextx = x;
    iter->y = iter->nexty = y;
    iter->spacing = fsctx->state.spacing;
    iter->str = str;
    iter->next = str;
    iter->end = end;
    iter->codepoint = 0;
    iter->prevGlyphIndex = -1;
    iter->bitmapOption = bitmapOption;

    return 1;

}

int fonsTextIterNext(struct fons_context *fsctx, struct fons_textiter *iter, struct fons_quad *quad)
{

    struct fons_glyph *glyph = 0;
    const char *str = iter->next;

    iter->str = iter->next;

    if (str == iter->end)
        return 0;

    for (; str != iter->end; str++)
    {

        if (decutf8(&iter->utf8state, &iter->codepoint, *(const unsigned char *)str))
            continue;

        str++;
        iter->x = iter->nextx;
        iter->y = iter->nexty;
        glyph = getGlyph(fsctx, iter->font, iter->codepoint, iter->size, iter->blur, iter->bitmapOption);

        if (glyph != 0)
            getQuad(fsctx, iter->font, iter->prevGlyphIndex, glyph, iter->scale, iter->spacing, &iter->nextx, &iter->nexty, quad);

        iter->prevGlyphIndex = glyph != 0 ? glyph->index : -1;

        break;

    }

    iter->next = str;

    return 1;

}

float fonsTextBounds(struct fons_context *fsctx, float x, float y, const char *str, const char *end, float *bounds)
{

    struct fons_font *font = fsctx->fonts[fsctx->state.font];
    float scale = tt_getPixelHeightScale(&font->font, fsctx->state.size);
    unsigned int codepoint;
    unsigned int utf8state = 0;
    struct fons_quad q;
    struct fons_glyph *glyph = 0;
    int prevGlyphIndex = -1;
    float startx, advance;
    float minx, miny, maxx, maxy;

    y += getVertAlign(fsctx, font, fsctx->state.align, fsctx->state.size);
    minx = maxx = x;
    miny = maxy = y;
    startx = x;

    for (; str != end; ++str)
    {

        if (decutf8(&utf8state, &codepoint, *(const unsigned char *)str))
            continue;

        glyph = getGlyph(fsctx, font, codepoint, fsctx->state.size, fsctx->state.blur, FONS_GLYPH_BITMAP_OPTIONAL);

        if (glyph != 0)
        {

            getQuad(fsctx, font, prevGlyphIndex, glyph, scale, fsctx->state.spacing, &x, &y, &q);

            if (q.x0 < minx)
                minx = q.x0;

            if (q.x1 > maxx)
                maxx = q.x1;

            if (fsctx->flags & FONS_ZERO_TOPLEFT)
            {

                if (q.y0 < miny)
                    miny = q.y0;

                if (q.y1 > maxy)
                    maxy = q.y1;

            }

            else
            {

                if (q.y1 < miny)
                    miny = q.y1;

                if (q.y0 > maxy)
                    maxy = q.y0;

            }

        }

        prevGlyphIndex = glyph != 0 ? glyph->index : -1;

    }

    advance = x - startx;

    if (fsctx->state.align & FONS_ALIGN_LEFT)
    {

    }

    else if (fsctx->state.align & FONS_ALIGN_RIGHT)
    {

        minx -= advance;
        maxx -= advance;

    }

    else if (fsctx->state.align & FONS_ALIGN_CENTER)
    {

        minx -= advance * 0.5f;
        maxx -= advance * 0.5f;

    }

    if (bounds)
    {

        bounds[0] = minx;
        bounds[1] = miny;
        bounds[2] = maxx;
        bounds[3] = maxy;

    }

    return advance;

}

void fonsVertMetrics(struct fons_context *fsctx, float *ascender, float *descender, float *lineh)
{

    struct fons_font *font = fsctx->fonts[fsctx->state.font];

    if (ascender)
        *ascender = font->ascender * fsctx->state.size;

    if (descender)
        *descender = font->descender * fsctx->state.size;

    if (lineh)
        *lineh = font->lineh * fsctx->state.size;

}

void fonsLineBounds(struct fons_context *fsctx, float y, float *miny, float *maxy)
{

    struct fons_font *font = fsctx->fonts[fsctx->state.font];

    y += getVertAlign(fsctx, font, fsctx->state.align, fsctx->state.size);

    if (fsctx->flags & FONS_ZERO_TOPLEFT)
    {

        *miny = y - font->ascender * fsctx->state.size;
        *maxy = *miny + font->lineh * fsctx->state.size;

    }

    else
    {

        *maxy = y + font->descender * fsctx->state.size;
        *miny = *maxy - font->lineh * fsctx->state.size;

    }

}

int fonsValidateTexture(struct fons_context *fsctx, int *dirty)
{

    if (fsctx->dirtyRect[0] < fsctx->dirtyRect[2] && fsctx->dirtyRect[1] < fsctx->dirtyRect[3])
    {

        dirty[0] = fsctx->dirtyRect[0];
        dirty[1] = fsctx->dirtyRect[1];
        dirty[2] = fsctx->dirtyRect[2];
        dirty[3] = fsctx->dirtyRect[3];
        fsctx->dirtyRect[0] = fsctx->width;
        fsctx->dirtyRect[1] = fsctx->height;
        fsctx->dirtyRect[2] = 0;
        fsctx->dirtyRect[3] = 0;

        return 1;

    }

    return 0;

}

void fons_delete(struct fons_context *fsctx)
{

    int i;

    for (i = 0; i < fsctx->nfonts; ++i)
        freeFont(fsctx->fonts[i]);

    if (fsctx->fonts)
        free(fsctx->fonts);

    if (fsctx->texData)
        free(fsctx->texData);

}

