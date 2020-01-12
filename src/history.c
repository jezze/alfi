#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "url.h"
#include "history.h"

static struct urlinfo stack[32];
static unsigned int historycount;

struct urlinfo *history_get(unsigned int index)
{

    return &stack[historycount - index - 1];

}

struct urlinfo *history_push(void)
{

    if (historycount < 31)
        historycount++;

    return history_get(0);

}

struct urlinfo *history_pop(void)
{

    if (historycount > 1)
        historycount--;

    return history_get(0);

}

char *history_geturl(unsigned int index)
{

    struct urlinfo *info = history_get(index);

    if (info)
        return info->url;

    return 0;

}

