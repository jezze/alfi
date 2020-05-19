#define GRIDFMT_HALIGN_LEFT             1
#define GRIDFMT_HALIGN_CENTER           2
#define GRIDFMT_HALIGN_RIGHT            3
#define GRIDFMT_VALIGN_TOP              1
#define GRIDFMT_VALIGN_MIDDLE           2
#define GRIDFMT_VALIGN_BOTTOM           3

unsigned int gridfmt_size(char *format);
unsigned int gridfmt_colsize(char *format, unsigned int index);
unsigned int gridfmt_colhalign(char *format, unsigned int index);
unsigned int gridfmt_colvalign(char *format, unsigned int index);
