struct parser
{

    struct
    {

        char *data;
        unsigned int count;
        unsigned int offset;
        unsigned int line;
        unsigned int linebreak;
        unsigned int inside;

    } expr;

    void (*fail)(unsigned int line);
    struct alfi_widget *(*findwidget)(char *name);
    struct alfi_widget *(*createwidget)(unsigned int type, char *in);
    struct alfi_widget *(*destroywidget)(struct alfi_widget *widget);
    char *(*allocate)(char *string, unsigned int size, unsigned int count, char *content);
    char *(*createstring)(char *string, char *content);
    char *(*destroystring)(char *string);

};

void parser_parse(struct parser *parser, char *in, unsigned int count, void *data);
void parser_init(struct parser *parser, void (*fail)(unsigned int line), struct alfi_widget *(*findwidget)(char *name), struct alfi_widget *(*createwidget)(unsigned int type, char *in), struct alfi_widget *(*destroywidget)(struct alfi_widget *widget), char *(*allocate)(char *string, unsigned int size, unsigned int count, char *content), char *(*createstring)(char *string, char *content), char *(*destroystring)(char *string));
