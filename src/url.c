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

void url_merge(char *out, char *current, char *url)
{

    if (strchr(url, ':'))
    {

        strcpy(out, url);

    }

    else
    {

        char *path;

        strcpy(out, current);

        path = strchr(out, ':') + 3;

        removepath(path);
        mergepath(out, url, url + strlen(url));

    }

}

