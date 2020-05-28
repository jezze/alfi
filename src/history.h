struct history
{

    char url[URL_SIZE];

};

struct history *history_get(unsigned int index);
struct history *history_push(void);
struct history *history_pop(void);
char *history_geturl(unsigned int index);
