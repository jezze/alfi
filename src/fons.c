#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "fons.h"

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
    fons_initstate(fsctx);

}

void fons_initstate(struct fons_context *fsctx)
{

    fsctx->state.size = 12.0f;
    fsctx->state.color = 0xffffffff;
    fsctx->state.font = 0;
    fsctx->state.spacing = 0;
    fsctx->state.align = FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE;

}

int fons_addfontfile(struct fons_context *fsctx, const char *path)
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

    return fons_addfontmem(fsctx, data, dataSize);

error:
    if (data)
        free(data);

    if (fp)
        fclose(fp);

    return -1;

}

int fons_addfontmem(struct fons_context *fsctx, unsigned char *data, int dataSize)
{

    int i, ascent, descent, fh, lineGap;
    struct fons_font *font = &fsctx->fonts[fsctx->nfonts];

    font->nglyphs = 0;

    for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
        font->lut[i] = -1;

    font->dataSize = dataSize;
    font->data = data;

    if (!stbtt_InitFont(&font->font, font->data, 0))
        return -1;

    stbtt_GetFontVMetrics(&font->font, &ascent, &descent, &lineGap);

    fh = ascent - descent;
    font->ascender = (float)ascent / (float)fh;
    font->descender = (float)descent / (float)fh;
    font->lineh = (float)(fh + lineGap) / (float)fh;

    fsctx->nfonts++;

    return fsctx->nfonts - 1;

}

static struct fons_glyph *getGlyph(struct fons_context *fsctx, struct fons_font *font, unsigned int codepoint, short size, int bitmapOption)
{

    int i, g, advance, lsb;
    int x0, y0, x1, y1;
    int gw, gh, gx, gy;
    int x, y;
    float scale;
    struct fons_glyph *glyph = 0;
    unsigned int h;
    int pad, added;
    unsigned char *dst;

    if (size < 2)
        return 0;

    pad = 2;
    h = hashint(codepoint) & (FONS_HASH_LUT_SIZE - 1);
    i = font->lut[h];

    while (i != -1)
    {
    
        if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == size)
        {

            glyph = &font->glyphs[i];

            if (bitmapOption == FONS_GLYPH_BITMAP_OPTIONAL || (glyph->x0 >= 0 && glyph->y0 >= 0))
                return glyph;

            break;

        }

        i = font->glyphs[i].next;

    }

    g = stbtt_FindGlyphIndex(&font->font, codepoint);

    if (!g)
        return 0;

    scale = stbtt_ScaleForPixelHeight(&font->font, size);

    stbtt_GetGlyphHMetrics(&font->font, g, &advance, &lsb);
    stbtt_GetGlyphBitmapBox(&font->font, g, scale, scale, &x0, &y0, &x1, &y1);

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

        glyph = &font->glyphs[font->nglyphs];
        glyph->codepoint = codepoint;
        glyph->size = size;
        glyph->next = 0;
        glyph->next = font->lut[h];
        font->lut[h] = font->nglyphs;
        font->nglyphs++;

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

    stbtt_MakeGlyphBitmap(&font->font, dst, gw - pad * 2, gh - pad * 2, fsctx->width, scale, scale, g);

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

        float adv = stbtt_GetGlyphKernAdvance(&font->font, prevGlyphIndex, glyph->index) * scale;

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

        rx = (int)(*x + xoff);
        ry = (int)(*y + yoff);
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

        rx = (int)(*x + xoff);
        ry = (int)(*y - yoff);
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

int fons_inititer(struct fons_context *fsctx, struct fons_textiter *iter, float x, float y, const char *str, const char *end, int bitmapOption)
{

    iter->font = &fsctx->fonts[fsctx->state.font];
    iter->size = fsctx->state.size;
    iter->scale = stbtt_ScaleForPixelHeight(&iter->font->font, iter->size);

    if (fsctx->state.align & FONS_ALIGN_LEFT)
    {

    }

    else if (fsctx->state.align & FONS_ALIGN_RIGHT)
    {

        x -= fons_getwidth(fsctx, x, y, str, end);

    }

    else if (fsctx->state.align & FONS_ALIGN_CENTER)
    {

        x -= fons_getwidth(fsctx, x, y, str, end) * 0.5f;

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
    iter->utf8state = 0;

    return 1;

}

int fons_nextiter(struct fons_context *fsctx, struct fons_textiter *iter, struct fons_quad *quad)
{

    const char *str = iter->next;

    iter->str = iter->next;

    if (str == iter->end)
        return 0;

    for (; str != iter->end; str++)
    {

        struct fons_glyph *glyph;

        if (decutf8(&iter->utf8state, &iter->codepoint, *(const unsigned char *)str))
            continue;

        str++;
        iter->x = iter->nextx;
        iter->y = iter->nexty;
        glyph = getGlyph(fsctx, iter->font, iter->codepoint, iter->size, iter->bitmapOption);

        if (glyph)
            getQuad(fsctx, iter->font, iter->prevGlyphIndex, glyph, iter->scale, iter->spacing, &iter->nextx, &iter->nexty, quad);

        iter->prevGlyphIndex = glyph != 0 ? glyph->index : -1;

        break;

    }

    iter->next = str;

    return 1;

}

float fons_getwidth(struct fons_context *fsctx, float x, float y, const char *str, const char *end)
{

    struct fons_font *font = &fsctx->fonts[fsctx->state.font];
    float scale = stbtt_ScaleForPixelHeight(&font->font, fsctx->state.size);
    unsigned int utf8state = 0;
    int prevGlyphIndex = -1;
    float startx = x;

    y += getVertAlign(fsctx, font, fsctx->state.align, fsctx->state.size);

    for (; str != end; ++str)
    {

        struct fons_glyph *glyph;
        struct fons_quad q;
        unsigned int codepoint;

        if (decutf8(&utf8state, &codepoint, *(const unsigned char *)str))
            continue;

        glyph = getGlyph(fsctx, font, codepoint, fsctx->state.size, FONS_GLYPH_BITMAP_OPTIONAL);

        if (glyph)
            getQuad(fsctx, font, prevGlyphIndex, glyph, scale, fsctx->state.spacing, &x, &y, &q);

        prevGlyphIndex = glyph != 0 ? glyph->index : -1;

    }

    return x - startx;

}

int fons_validate(struct fons_context *fsctx, int *dirty)
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
    {

        if (fsctx->fonts[i].data)
            free(fsctx->fonts[i].data);

    }

    if (fsctx->texData)
        free(fsctx->texData);

}

