#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"
#include "url.h"
#include "resource.h"

void resource_save(struct resource *resource, unsigned int count, void *data)
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

unsigned int resource_load(struct resource *resource, unsigned int count, void *data)
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
            resource_save(resource, bcount, bdata);

        close(fd[0]);

        return resource->count;

    }

    else
    {

        dup2(fd[0], 0);
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);

        if (count)
            execlp("navi-resolve", "navi-resolve", resource->urlinfo.url, data, NULL);
        else
            execlp("navi-resolve", "navi-resolve", resource->urlinfo.url, NULL);

        exit(EXIT_FAILURE);

        return 0;

    }

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

