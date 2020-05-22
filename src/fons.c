#include <stdlib.h>
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

static int atlasinsertnode(struct fons_atlas *atlas, int idx, int x, int y, int w)
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

static void atlasremovenode(struct fons_atlas *atlas, int idx)
{

    int i;

    if (!atlas->nnodes)
        return;

    for (i = idx; i < atlas->nnodes - 1; i++)
        atlas->nodes[i] = atlas->nodes[i + 1];

    atlas->nnodes--;

}

static int atlasaddskylinelevel(struct fons_atlas *atlas, int idx, int x, int y, int w, int h)
{

    int i;

    if (!atlasinsertnode(atlas, idx, x, y + h, w))
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

                atlasremovenode(atlas, i);

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

            atlasremovenode(atlas, i + 1);

            i--;

        }

    }

    return 1;

}

static int atlasrectfits(struct fons_atlas *atlas, int i, int w, int h)
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

static int atlasaddrect(struct fons_atlas *atlas, int rw, int rh, int *rx, int *ry)
{

    int besth = atlas->height;
    int bestw = atlas->width;
    int besti = -1;
    int bestx = -1;
    int besty = -1;
    unsigned int i;

    for (i = 0; i < atlas->nnodes; i++)
    {

        int y = atlasrectfits(atlas, i, rw, rh);

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

    if (!atlasaddskylinelevel(atlas, besti, bestx, besty, rw, rh))
        return 0;

    *rx = bestx;
    *ry = besty;

    return 1;

}

void fons_create(struct fons_context *fsctx, int width, int height)
{

    fsctx->width = width;
    fsctx->height = height;
    fsctx->atlas.width = width;
    fsctx->atlas.height = height;
    fsctx->atlas.nodes[0].x = 0;
    fsctx->atlas.nodes[0].y = 0;
    fsctx->atlas.nodes[0].width = width;
    fsctx->atlas.nnodes = 1;
    fsctx->nfonts = 0;
    fsctx->texdata = malloc(fsctx->width * fsctx->height);

    if (!fsctx->texdata)
        return;

    memset(fsctx->texdata, 0, fsctx->width * fsctx->height);

}

int fons_addfont(struct fons_context *fsctx, unsigned char *data, unsigned int count)
{

    int i, ascent, descent, fh, lineGap;
    struct fons_font *font = &fsctx->fonts[fsctx->nfonts];

    font->nglyphs = 0;

    for (i = 0; i < FONS_HASH_LUT_SIZE; ++i)
        font->lut[i] = -1;

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

struct fons_glyph *fons_getglyph(struct fons_context *fsctx, struct fons_font *font, unsigned int codepoint, short size)
{

    int i, g, advance, lsb;
    int x0, y0, x1, y1;
    int gw, gh, gx, gy;
    int x, y;
    float scale;
    struct fons_glyph *glyph = 0;
    unsigned int h;
    int pad = 2;
    int added;
    unsigned char *dst;

    if (size < 2)
        return 0;

    h = hashint(codepoint) & (FONS_HASH_LUT_SIZE - 1);
    i = font->lut[h];

    while (i != -1)
    {
    
        if (font->glyphs[i].codepoint == codepoint && font->glyphs[i].size == size)
        {

            glyph = &font->glyphs[i];

            if (glyph->x0 >= 0 && glyph->y0 >= 0)
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

    added = atlasaddrect(&fsctx->atlas, gw, gh, &gx, &gy);

    if (!added)
        return 0;

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

    dst = &fsctx->texdata[(glyph->x0 + pad) + (glyph->y0 + pad) * fsctx->width];

    stbtt_MakeGlyphBitmap(&font->font, dst, gw - pad * 2, gh - pad * 2, fsctx->width, scale, scale, g);

    dst = &fsctx->texdata[glyph->x0 + glyph->y0 * fsctx->width];

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

    return glyph;

}

float getglyphwidth(struct fons_context *fsctx, struct fons_font *font, int codepoint, float size)
{

    struct fons_glyph *glyph = fons_getglyph(fsctx, font, codepoint, size);

    return (glyph) ? (int)(glyph->xadv) : 0;

}

static float getstringwidth(struct fons_context *fsctx, struct fons_font *font, float size, float x, const char *str, const char *end)
{

    unsigned int utf8state = 0;
    float startx = x;

    for (; str != end; ++str)
    {

        unsigned int codepoint;

        if (decutf8(&utf8state, &codepoint, *(const unsigned char *)str))
            continue;

        x += getglyphwidth(fsctx, font, codepoint, size);

    }

    return x - startx;

}

void fons_getquad(struct fons_context *fsctx, struct fons_glyph *glyph, struct fons_quad *quad, float x, float y)
{

    float xoff = glyph->xoff + 1;
    float yoff = glyph->yoff + 1;
    float x0 = glyph->x0 + 1;
    float y0 = glyph->y0 + 1;
    float x1 = glyph->x1 - 1;
    float y1 = glyph->y1 - 1;
    float itw = 1.0f / fsctx->width;
    float ith = 1.0f / fsctx->height;
    float rx = (int)(x + xoff);
    float ry = (int)(y + yoff);

    quad->x0 = rx;
    quad->y0 = ry;
    quad->x1 = rx + x1 - x0;
    quad->y1 = ry + y1 - y0;
    quad->s0 = x0 * itw;
    quad->t0 = y0 * ith;
    quad->s1 = x1 * itw;
    quad->t1 = y1 * ith;

}

int fons_inititer(struct fons_context *fsctx, struct fons_textiter *iter, struct fons_font *font, int align, float size, float x, float y, const char *str, const char *end)
{

    iter->font = font;
    iter->size = size;

    if (align & FONS_ALIGN_LEFT)
        iter->x = x;
    else if (align & FONS_ALIGN_RIGHT)
        iter->x = x - getstringwidth(fsctx, iter->font, iter->size, x, str, end);
    else if (align & FONS_ALIGN_CENTER)
        iter->x = x - getstringwidth(fsctx, iter->font, iter->size, x, str, end) * 0.5f;

    if (align & FONS_ALIGN_BASELINE)
        iter->y = y;
    else if (align & FONS_ALIGN_TOP)
        iter->y = y + iter->font->ascender * size;
    else if (align & FONS_ALIGN_MIDDLE)
        iter->y = y + (iter->font->ascender + iter->font->descender) / 2.0f * size;
    else if (align & FONS_ALIGN_BOTTOM)
        iter->y = y + iter->font->descender * size;

    iter->nextx = iter->x;
    iter->nexty = iter->y;
    iter->str = str;
    iter->next = str;
    iter->end = end;
    iter->codepoint = 0;
    iter->utf8state = 0;

    return 1;

}

int fons_nextiter(struct fons_context *fsctx, struct fons_textiter *iter)
{

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
        iter->nextx += getglyphwidth(fsctx, iter->font, iter->codepoint, iter->size);

        break;

    }

    iter->next = str;

    return 1;

}

void fons_delete(struct fons_context *fsctx)
{

    if (fsctx->texdata)
        free(fsctx->texdata);

}

