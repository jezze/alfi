struct urlinfo *history_get(unsigned int index);
struct urlinfo *history_push(void);
struct urlinfo *history_pop(void);
char *history_geturl(unsigned int index);
