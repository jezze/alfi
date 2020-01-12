#define URL_SIZE                        0x800

struct urlinfo
{

    char url[URL_SIZE];

};

void url_set(struct urlinfo *info, char *url);
void url_unset(struct urlinfo *info);
void url_merge(struct urlinfo *info, char *current, char *url);
