#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "url.h"
#include "resource.h"
#include "history.h"

static void save(struct resource *resource, unsigned int count, void *data)
{

    unsigned int free = resource->size - resource->count;

    if (free < count)
    {

        resource->size += RESOURCE_PAGESIZE;
        resource->data = realloc(resource->data, resource->size);

    }

    memcpy((char *)resource->data + resource->count, data, count);

    resource->count += count;

}

static unsigned int resolve(struct resource *resource, const char *name, char * const *args)
{

    int fd[2];
    pid_t pid;

    if (pipe(fd) < 0)
        return 0;

    pid = fork();

    if (pid < 0)
        return 0;

    if (pid)
    {

        char bdata[RESOURCE_PAGESIZE];
        int bcount;

        close(fd[1]);

        while ((bcount = read(fd[0], bdata, RESOURCE_PAGESIZE)))
            save(resource, bcount, bdata);

        close(fd[0]);

        return resource->count;

    }

    else
    {

        dup2(fd[0], 0);
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execvp(name, args);
        exit(EXIT_FAILURE);

        return 0;

    }

}

static unsigned int _curl_match(struct resource *resource)
{

    return !strncmp(resource->url, "http:", 5) || !strncmp(resource->url, "https:", 6);

}

static unsigned int _curl_load(struct resource *resource, unsigned int count, void *data)
{

    char *q[16];

    if (count)
    {

        q[0] = "curl";
        q[1] = "-s";
        q[2] = "-L";
        q[3] = "-A";
        q[4] = "Navi/1.0";
        q[5] = "-X";
        q[6] = "POST";
        q[7] = "-H";
        q[8] = "Content-Type: application/x-www-form-urlencoded";
        q[9] = "-d";
        q[10] = data;
        q[11] = resource->url;
        q[12] = 0;


    }

    else
    {

        q[0] = "curl";
        q[1] = "-s";
        q[2] = "-L";
        q[3] = "-A";
        q[4] = "Navi/1.0";
        q[5] = "-X";
        q[6] = "GET";
        q[7] = resource->url;
        q[8] = 0;

    }

    return resolve(resource, "curl", q);

}

static unsigned int _navi_match(struct resource *resource)
{

    return !strncmp(resource->url, "navi:", 5);

}

static unsigned int _navi_load(struct resource *resource, unsigned int count, void *data)
{

    char *path = resource->url + 7;

    if (!strncmp(path, "blank", 5))
    {

        char buffer[4096];
        static char *fmt =
            "= window label \"Navi 1.0\"\n"
            "+ table id bar grid \"08:04\"\n"
            "+ field id \"url\" in bar label \"URL\" data \"%s\" onlinebreak \"post\" \"navi://lookup\"\n"
            "+ button in bar label \"Lookup\" onclick \"post\" \"navi://lookup\" mode \"on\" icon \"earth\"\n"
            "+ text label \"Notice: Use the right mouse button to go back.\"\n"
            "+ divider\n"
            "+ header2 label \"Bookmarks\"\n"
            "+ text label \"No bookmarks\"\n"
            "+ divider\n"
            "+ button label \"Settings\" onclick \"get\" \"navi://settings\" icon \"options\"\n";

        resource->count = sprintf(buffer, fmt, "http://");
        resource->data = malloc(resource->count);

        strcpy(resource->data, buffer);

        return resource->count;

    }

    else if (!strncmp(path, "settings", 8))
    {

        char buffer[4096];
        static char *fmt =
            "= window label \"Settings\"\n"
            "+ header label \"Settings\"\n"
            "+ table id settings grid \"06\"\n"
            "+ text in settings label \"Left mouse button\"\n"
            "+ text in settings label \"Interact\"\n"
            "+ text in settings label \"Right mouse button\"\n"
            "+ text in settings label \"Go back\"\n"
            "+ text in settings label \"Middle mouse button\"\n"
            "+ text in settings label \"Go home\"\n"
            "+ text in settings label \"Ctrl+B *\"\n"
            "+ text in settings label \"Bookmarks view\"\n"
            "+ text in settings label \"Ctrl+D\"\n"
            "+ text in settings label \"File view\"\n"
            "+ text in settings label \"Ctrl+H *\"\n"
            "+ text in settings label \"History view\"\n"
            "+ text in settings label \"Ctrl+L\"\n"
            "+ text in settings label \"Home\"\n"
            "+ text in settings label \"Ctrl+M\"\n"
            "+ text in settings label \"Night mode\"\n"
            "+ text in settings label \"Ctrl+N\"\n"
            "+ text in settings label \"Light mode\"\n"
            "+ text in settings label \"Ctrl+Q\"\n"
            "+ text in settings label \"Quit\"\n"
            "+ text in settings label \"Ctrl+R\"\n"
            "+ text in settings label \"Refresh\"\n"
            "+ text in settings label \"Ctrl+V\"\n"
            "+ text in settings label \"Paste from clipboard\"\n"
            "+ text label \"* = Needs certain setup to work.\"\n";

        resource->count = sprintf(buffer, fmt, data);
        resource->data = malloc(resource->count);

        strcpy(resource->data, buffer);

        return resource->count;

    }

    else if (!strncmp(path, "notfound", 8))
    {

        char buffer[4096];
        static char *fmt =
            "= window label \"Not found\"\n"
            "+ header label \"Not found\"\n"
            "+ text label \"The URL does not seem to exist.\"\n"
            "+ code label \"URL: %s\"\n"
            "+ text label \"Press the right mouse button to go back.\"\n";

        resource->count = sprintf(buffer, fmt, data);
        resource->data = malloc(resource->count);

        strcpy(resource->data, buffer);

        return resource->count;

    }

    else if (!strncmp(path, "syntaxerror", 8))
    {

        char buffer[4096];
        static char *fmt =
            "= window label \"Syntax error\"\n"
            "+ header label \"Syntax error\"\n"
            "+ text label \"This page could not be understood. Maybe it is not an ALFI website?\"\n"
            "+ code label \"URL: %s\"\n"
            "+ text label \"Press the right mouse button to go back.\"\n";

        resource->count = sprintf(buffer, fmt, data);
        resource->data = malloc(resource->count);

        strcpy(resource->data, buffer);

        return resource->count;

    }

    else if (!strncmp(path, "error", 5))
    {

        char buffer[4096];
        static char *fmt =
            "= window label \"Internal error\"\n"
            "+ header label \"Internal error\"\n"
            "+ text label \"An internal error occured.\"\n";

        resource->count = sprintf(buffer, fmt);
        resource->data = malloc(resource->count);

        strcpy(resource->data, buffer);

        return resource->count;

    }

    else if (!strncmp(path, "lookup", 6))
    {

        struct history *current = history_get(0);
        char *urlparam = data;

        strcpy(resource->url, urlparam + 4);
        strcpy(current->url, resource->url);
        resource_load(resource, 0, 0);

        return resource->count;

    }

    return 0;

}

static unsigned int _external_match(struct resource *resource)
{

    return 1;

}

static unsigned int _external_load(struct resource *resource, unsigned int count, void *data)
{

    char *q[4];

    q[0] = "navi-resolve";
    q[1] = resource->url;
    q[2] = data;
    q[3] = 0;

    return resolve(resource, "navi-resolve", q);

}

void resource_load(struct resource *resource, unsigned int count, void *data)
{

    if (_navi_match(resource))
        _navi_load(resource, count, data);
    else if (_curl_match(resource))
        _curl_load(resource, count, data);
    else if (_external_match(resource))
        _external_load(resource, count, data);

}

void resource_unload(struct resource *resource)
{

    if (resource->data)
        free(resource->data);

}

unsigned int resource_iref(struct resource *resource)
{

    return ++resource->refcount;

}

unsigned int resource_dref(struct resource *resource)
{

    return --resource->refcount;

}

void resource_init(struct resource *resource, char *url)
{

    strcpy(resource->url, url);

    resource->data = 0;
    resource->size = 0;
    resource->count = 0;
    resource->refcount = 0;

}

void resource_destroy(struct resource *resource)
{

    strcpy(resource->url, "");

    resource->data = 0;
    resource->size = 0;
    resource->count = 0;
    resource->refcount = 0;

}

