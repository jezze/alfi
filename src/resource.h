struct resource
{

    char url[1024];
    void *data;
    unsigned int size;

};

void resource_seturl(struct resource *resource, char *url);
