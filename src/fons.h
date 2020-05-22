#define FONS_HASH_LUT_SIZE 256
#define FONS_INIT_FONTS 8
#define FONS_INIT_GLYPHS 256
#define FONS_INIT_ATLAS_NODES 256

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

struct fons_quad
{

    float x0, y0, s0, t0;
    float x1, y1, s1, t1;

};

struct fons_textiter
{

    float x, y;
    float nextx, nexty;
    unsigned int codepoint;
    short size;
    struct fons_font *font;
    const char *str;
    const char *next;
    const char *end;
    unsigned int utf8state;

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
    float ascender;
    float descender;
    float lineh;
    struct fons_glyph glyphs[FONS_INIT_GLYPHS];
    unsigned int nglyphs;
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
    unsigned int nnodes;

};

struct fons_context
{

    int width, height;
    unsigned char *texdata;
    struct fons_atlas atlas;
    struct fons_font fonts[FONS_INIT_FONTS];
    unsigned int nfonts;

};

void fons_create(struct fons_context *fsctx, int width, int height);
void fons_delete(struct fons_context *fsctx);
int fons_addfont(struct fons_context *fsctx, unsigned char *data, unsigned int count);
struct fons_glyph *fons_getglyph(struct fons_context *fsctx, struct fons_font *font, unsigned int codepoint, short size);
void fons_getquad(struct fons_context *fsctx, struct fons_glyph *glyph, struct fons_quad *quad, float x, float y);
int fons_inititer(struct fons_context *fsctx, struct fons_textiter *iter, struct fons_font *font, int align, float size, float x, float y, const char *str, const char *end);
int fons_nextiter(struct fons_context *fsctx, struct fons_textiter *iter);
