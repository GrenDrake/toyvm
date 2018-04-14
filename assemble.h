#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#define HEADER_SIZE 12


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


struct label_def {
    char *name;
    int pos;
    struct label_def *next;
};


struct mnemonic {
    int opcode;
    const char *name;
    int operand_size;
};



char *str_dup(const char *source);

struct token_list* init_token_list(void);
void add_token(struct token_list *list, struct token *new_token);
void free_tokens(struct token_list *list);
void dump_tokens(struct token_list *list);
struct token_list* lex_file(const char *filename);

int add_label(struct label_def **first_lbl, const char *name, int pos);
struct label_def* get_label(struct label_def *first, const char *name);
void dump_labels(struct label_def *first);
void free_labels(struct label_def *first);

int parse_tokens(struct token_list *list, const char *output_filename);

#endif
