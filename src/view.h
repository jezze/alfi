struct view
{

    int pagew;
    int pageh;
    int unitw;
    int unith;
    int marginw;
    int marginh;
    int scrollx;
    int scrolly;
    int padw;
    int padh;
    int fontsizesmall;
    int fontsizemedium;
    int fontsizelarge;
    int fontsizexlarge;

};

void view_init(struct view *view, int w, int h, unsigned int size);
void view_fontsize(struct view *view, unsigned int size);
void view_reset(struct view *view);
void view_scroll(struct view *view, int x, int y);
void view_adjust(struct view *view, float w, float h);
