#ifndef ASSEMBLE_H
#define ASSEMBLE_H

enum token_type {
    tt_bad,
    tt_identifier,
    tt_integer,
    tt_string,
    tt_colon,
    tt_eol
};

struct token {
    const char *source_file;
    int line, column;

    enum token_type type;
    char *text;
    int i;

    struct token *next;
};

struct token_list {
    struct token *first;
    struct token *last;
};

char *str_dup(const char *source);

struct token_list* init_token_list(void);
void add_token(struct token_list *list, struct token *new_token);
void free_tokens(struct token_list *list);
void dump_tokens(struct token_list *list);
struct token_list* lex_file(const char *filename);

int parse_tokens(struct token_list *list);

#endif
