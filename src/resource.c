#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "url.h"
#include "resource.h"

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

    return !strncmp(resource->urlinfo.url, "http:", 5) || !strncmp(resource->urlinfo.url, "https:", 6);

}

static unsigned int _curl_load(struct resource *resource, unsigned int count, void *data)
{

    char *q[12];

    if (count)
    {

        q[0] = "curl";
        q[1] = "-s";
        q[2] = "-A";
        q[3] = "Navi/1.0";
        q[4] = "-X";
        q[5] = "POST";
        q[6] = "-H";
        q[7] = "Content-Type: application/x-www-form-urlencoded";
        q[8] = "-d";
        q[9] = data;
        q[10] = resource->urlinfo.url;
        q[11] = 0;


    }

    else
    {

        q[0] = "curl";
        q[1] = "-s";
        q[2] = "-A";
        q[3] = "Navi/1.0";
        q[4] = "-X";
        q[5] = "GET";
        q[6] = resource->urlinfo.url;
        q[7] = 0;

    }

    return resolve(resource, "curl", q);

}

static unsigned int _navi_match(struct resource *resource)
{

    return !strncmp(resource->urlinfo.url, "navi:", 5);

}

static unsigned int _navi_load(struct resource *resource, unsigned int count, void *data)
{

    char *path = resource->urlinfo.url + 7;

    if (!strncmp(path, "blank", 5))
    {

        char buffer[4096];
        static char *fmt =
            "= window label \"Lookup address\"\n"
            "+ header label \"Lookup address\"\n"
            "+ field id \"url\" label \"URL\" data \"%s\"\n"
            "+ button label \"Lookup\" link \"navi://lookup\" \"text/alfi\" mode \"on\"\n"
            "+ subheader label \"Quick links\"\n"
            "+ anchor label \"blunder.se\" link \"http://www.blunder.se/\" \"text/alfi\"\n"
            "+ anchor label \"example\" link \"file:///usr/share/navi/example.alfi\" \"text/alfi\"\n";

        resource->count = sprintf(buffer, fmt, "http://");
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

        char *urlparam = data;

        url_set(&resource->urlinfo, urlparam + 4);

        return resource_load(resource, 0, 0);

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
    q[1] = resource->urlinfo.url;
    q[2] = data;
    q[3] = 0;

    return resolve(resource, "navi-resolve", q);

}

unsigned int resource_load(struct resource *resource, unsigned int count, void *data)
{

    if (_navi_match(resource))
        return _navi_load(resource, count, data);
    else if (_curl_match(resource))
        return _curl_load(resource, count, data);
    else if (_external_match(resource))
        return _external_load(resource, count, data);

    return 0;

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

    url_set(&resource->urlinfo, url);

    resource->data = 0;
    resource->size = 0;
    resource->count = 0;
    resource->refcount = 0;

}

void resource_destroy(struct resource *resource)
{

    url_unset(&resource->urlinfo);

    if (resource->data)
        free(resource->data);

    resource->data = 0;
    resource->size = 0;
    resource->count = 0;
    resource->refcount = 0;

}

