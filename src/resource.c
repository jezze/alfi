#include <stdlib.h>
#include <string.h>
#include "resource.h"

void resource_seturl(struct resource *resource, char *url)
{

    if (resource->url != url)
        strcpy(resource->url, url);

}

