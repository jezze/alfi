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

    struct
    {

        char *data;
        unsigned int count;
        unsigned int offset;

    } string;

    void (*fail)(unsigned int line);
    struct alfi_widget *(*find)(char *name, unsigned int group);
    struct alfi_widget *(*create)(unsigned int type, unsigned int group, char *in);
    void (*destroy)(struct alfi_widget *widget);

};

void parse(struct parser *parser, unsigned int group, char *in);
