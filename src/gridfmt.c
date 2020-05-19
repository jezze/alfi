#include <stdlib.h>
#include <string.h>
#include "gridfmt.h"

static unsigned int copycolumn(char *coldata, char *format)
{

    unsigned int length = strlen(format);
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        if (format[i] == ':')
            break;

        coldata[i] = format[i];

    }

    coldata[i] = '\0';

    return i;

}

static unsigned int getcolumn(char *coldata, unsigned int index, char *format)
{

    unsigned int length = strlen(format);
    unsigned int i;

    if (!index)
        return copycolumn(coldata, format);

    for (i = 0; i < length; i++)
    {

        if (format[i] == ':')
        {

            if (!--index)
                return copycolumn(coldata, format + i + 1);

        }

    }

    return 0;

}

unsigned int gridfmt_size(char *format)
{

    unsigned int length = strlen(format);
    unsigned int size = 0;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        if (format[i] == ':')
            size++;

    }

    return size + 1;

}

unsigned int gridfmt_colsize(char *format, unsigned int index)
{

    char coldata[8];
    unsigned int count = getcolumn(coldata, index, format);
    unsigned int colsize = 12;

    if (count >= 2)
        colsize = 10 * (coldata[0] - '0') + (coldata[1] - '0');

    if (colsize > 12)
        colsize = 12;

    return colsize;

}

unsigned int gridfmt_colhalign(char *format, unsigned int index)
{

    char coldata[8];
    unsigned int count = getcolumn(coldata, index, format);

    if (count >= 3)
    {

        switch (coldata[2])
        {

        case 'L':
            return GRIDFMT_HALIGN_LEFT;

        case 'R':
            return GRIDFMT_HALIGN_RIGHT;

        case 'C':
            return GRIDFMT_HALIGN_CENTER;

        }

    }

    return GRIDFMT_HALIGN_LEFT;

}

unsigned int gridfmt_colvalign(char *format, unsigned int index)
{

    char coldata[8];
    unsigned int count = getcolumn(coldata, index, format);

    if (count >= 4)
    {

        switch (coldata[3])
        {

        case 'T':
            return GRIDFMT_VALIGN_TOP;

        case 'B':
            return GRIDFMT_VALIGN_BOTTOM;

        case 'M':
            return GRIDFMT_VALIGN_MIDDLE;

        }

    }

    return GRIDFMT_VALIGN_TOP;

}

