struct view
{

    unsigned int pagew;
    unsigned int pageh;
    unsigned int unitw;
    unsigned int unith;
    unsigned int marginw;
    unsigned int marginh;
    unsigned int scrollw;
    unsigned int scrollh;
    int scrollx;
    int scrolly;
    unsigned int fontsizesmall;
    unsigned int fontsizemedium;
    unsigned int fontsizelarge;
    unsigned int fontsizexlarge;

};

void view_init(struct view *view, int w, int h);
void view_reset(struct view *view);
void view_scroll(struct view *view, int x, int y);
void view_adjust(struct view *view, float w, float h);
