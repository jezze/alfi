#include <stdlib.h>
#include <string.h>
#include "url.h"

static void removepath(char *buffer)
{

    char *c = strrchr(buffer, '/');

    if (!c)
        return;

    c[0] = '\0';

}

static void mergesimple(char *buffer, char *path, unsigned int count)
{

    if (count == 2 && path[0] == '.' && path[1] == '.')
    {

        removepath(buffer);

    }

    else if (count == 1 && path[0] == '.')
    {

    }

    else
    {

        strcat(buffer, "/");
        strncat(buffer, path, count);

    }

}

static void mergepath(char *buffer, char *path, char *end)
{

    char *p;
    char *c;

    for (p = path; (c = strchr(p, '/')); p = c + 1)
        mergesimple(buffer, p, c - p);

    if (p < end)
        mergesimple(buffer, p, end - p);

}

void url_set(struct urlinfo *info, char *url)
{

    if (info->url != url)
        strcpy(info->url, url);

}

void url_unset(struct urlinfo *info)
{

    info->url[0] = '\0';

}

void url_merge(struct urlinfo *info, char *current, char *url)
{

    if (strchr(url, ':'))
    {

        url_set(info, url);

    }

    else
    {

        url_set(info, current);
        removepath(info->url);
        mergepath(info->url, url, url + strlen(url));

    }

}

