#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "attributes.h"
#include "pool.h"

void attribute_data_create(struct attribute_data *attribute, char *content)
{

    attribute->total = ATTRIBUTE_DATASIZE;
    attribute->offset = strlen(content);
    attribute->content = pool_allocate(ATTRIBUTE_TYPE_DATA, attribute->content, attribute->total, attribute->offset + 1, content);

}

void attribute_data_destroy(struct attribute_data *attribute)
{

    attribute->content = pool_string_destroy(ATTRIBUTE_TYPE_DATA, attribute->content);
    attribute->total = 0;
    attribute->offset = 0;

}

void attribute_grid_create(struct attribute_grid *attribute, char *format)
{

    attribute->format = pool_string_create(ATTRIBUTE_TYPE_GRID, attribute->format, format);

}

void attribute_grid_destroy(struct attribute_grid *attribute)
{

    attribute->format = pool_string_destroy(ATTRIBUTE_TYPE_GRID, attribute->format);

}

void attribute_icon_create(struct attribute_icon *attribute, unsigned int type)
{

    attribute->type = type;

}

void attribute_icon_destroy(struct attribute_icon *attribute)
{

    attribute->type = 0;

}

void attribute_id_create(struct attribute_id *attribute, char *name)
{

    attribute->name = pool_string_create(ATTRIBUTE_TYPE_ID, attribute->name, name);

}

void attribute_id_destroy(struct attribute_id *attribute)
{

    attribute->name = pool_string_destroy(ATTRIBUTE_TYPE_ID, attribute->name);

}

void attribute_in_create(struct attribute_in *attribute, char *name)
{

    attribute->name = pool_string_create(ATTRIBUTE_TYPE_IN, attribute->name, name);

}

void attribute_in_destroy(struct attribute_in *attribute)
{

    attribute->name = pool_string_destroy(ATTRIBUTE_TYPE_IN, attribute->name);

}

void attribute_label_create(struct attribute_label *attribute, char *content)
{

    attribute->content = pool_string_create(ATTRIBUTE_TYPE_LABEL, attribute->content, content);

}

void attribute_label_destroy(struct attribute_label *attribute)
{

    attribute->content = pool_string_destroy(ATTRIBUTE_TYPE_LABEL, attribute->content);

}

void attribute_link_create(struct attribute_link *attribute, char *url, char *mime)
{

    attribute->url = pool_string_create(ATTRIBUTE_TYPE_LINK, attribute->url, url);
    attribute->mime = pool_string_create(ATTRIBUTE_TYPE_LINK, attribute->mime, mime);

}

void attribute_link_destroy(struct attribute_link *attribute)
{

    attribute->url = pool_string_destroy(ATTRIBUTE_TYPE_LINK, attribute->url);
    attribute->mime = pool_string_destroy(ATTRIBUTE_TYPE_LINK, attribute->mime);

}

void attribute_mode_create(struct attribute_mode *attribute, unsigned int type)
{

    attribute->type = type;

}

void attribute_mode_destroy(struct attribute_mode *attribute)
{

    attribute->type = 0;

}

void attribute_range_create(struct attribute_range *attribute, unsigned int min, unsigned int max)
{

    attribute->min = min;
    attribute->max = max;

}

void attribute_range_destroy(struct attribute_range *attribute)
{

    attribute->min = 0;
    attribute->max = 0;

}

void attribute_target_create(struct attribute_target *attribute, unsigned int type)
{

    attribute->type = type;

}

void attribute_target_destroy(struct attribute_target *attribute)
{

    attribute->type = 0;

}

void attribute_type_create(struct attribute_type *attribute, unsigned int type)
{

    attribute->type = type;

}

void attribute_type_destroy(struct attribute_type *attribute)
{

    attribute->type = 0;

}

