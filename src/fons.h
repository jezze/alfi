#define FONS_SCRATCH_BUF_SIZE 96000
#define FONS_HASH_LUT_SIZE 256
#define FONS_INIT_FONTS 4
#define FONS_INIT_GLYPHS 256
#define FONS_INIT_ATLAS_NODES 256
#define FONS_VERTEX_COUNT 1024

enum fons_flags
{

    FONS_ZERO_TOPLEFT = 1,
    FONS_ZERO_BOTTOMLEFT = 2

};

enum fons_align
{

    FONS_ALIGN_LEFT     = 1 << 0,
    FONS_ALIGN_CENTER   = 1 << 1,
    FONS_ALIGN_RIGHT    = 1 << 2,
    FONS_ALIGN_TOP      = 1 << 3,
    FONS_ALIGN_MIDDLE   = 1 << 4,
    FONS_ALIGN_BOTTOM   = 1 << 5,
    FONS_ALIGN_BASELINE = 1 << 6

};

enum fons_glyphbitmap
{

    FONS_GLYPH_BITMAP_OPTIONAL = 1,
    FONS_GLYPH_BITMAP_REQUIRED = 2

};

struct fons_quad
{

    float x0, y0, s0, t0;
    float x1, y1, s1, t1;

};

struct fons_textiter
{

    float x, y;
    float nextx, nexty;
    float scale;
    float spacing;
    unsigned int codepoint;
    short size;
    struct fons_font *font;
    int prevGlyphIndex;
    const char *str;
    const char *next;
    const char *end;
    unsigned int utf8state;
    int bitmapOption;

};

struct fons_glyph
{

    unsigned int codepoint;
    int index;
    int next;
    short size;
    short x0, y0, x1, y1;
    short xadv, xoff, yoff;

};

struct fons_font
{

    stbtt_fontinfo font;
    unsigned char *data;
    int dataSize;
    float ascender;
    float descender;
    float lineh;
    struct fons_glyph glyphs[FONS_INIT_GLYPHS];
    int nglyphs;
    int lut[FONS_HASH_LUT_SIZE];

};

struct fons_atlasnode
{

    short x, y;
    short width;

};

struct fons_atlas
{

    int width, height;
    struct fons_atlasnode nodes[FONS_INIT_ATLAS_NODES];
    int nnodes;

};

struct fons_state
{

    int font;
    int align;
    float size;
    unsigned int color;
    float spacing;

};

struct fons_context
{

    int width, height;
    unsigned char flags;
    float itw, ith;
    unsigned char *texData;
    int dirtyRect[4];
    struct fons_atlas atlas;
    struct fons_font fonts[FONS_INIT_FONTS];
    int nfonts;
    float verts[FONS_VERTEX_COUNT * 2];
    int nverts;
    float tcoords[FONS_VERTEX_COUNT * 2];
    unsigned int colors[FONS_VERTEX_COUNT];
    struct fons_state state;

};

void fons_create(struct fons_context *fsctx, int width, int height, unsigned char flags);
void fons_delete(struct fons_context *fsctx);
int fons_addfontfile(struct fons_context *fsctx, const char *path);
int fons_addfontmem(struct fons_context *fsctx, unsigned char *data, int ndata);
void fons_initstate(struct fons_context *fsctx);
float fons_getwidth(struct fons_context *fsctx, float x, float y, const char *string, const char *end);
int fons_inititer(struct fons_context *fsctx, struct fons_textiter *iter, float x, float y, const char *str, const char *end, int bitmapOption);
int fons_nextiter(struct fons_context *fsctx, struct fons_textiter *iter, struct fons_quad *quad);
int fons_validate(struct fons_context *fsctx, int *dirty);
