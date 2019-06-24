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
    struct alfi_widget *(*find)(char *name, unsigned int group);
    struct alfi_widget *(*create)(unsigned int type, unsigned int group, char *in);
    void (*destroy)(struct alfi_widget *widget);
    char *(*createstring)(unsigned int size);
    void (*destroystring)(char *string);

};

void parser_parse(struct parser *parser, unsigned int group, char *in);
void parser_parsedata(struct parser *parser, unsigned int group, char *in, unsigned int count, void *data);
void parser_init(struct parser *parser, void (*fail)(unsigned int line), struct alfi_widget *(*find)(char *name, unsigned int group), struct alfi_widget *(*create)(unsigned int type, unsigned int group, char *in), void (*destroy)(struct alfi_widget *widget), char *(*createstring)(unsigned int size), void (*destroystring)(char *string));
