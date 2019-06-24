#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdint.h>
#include <stdio.h>

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


struct map_line {
    char *data;
    struct map_line *next;
};

struct map_data {
    unsigned size;
    unsigned width, height;
    struct map_line *data;
};

struct backpatch {
    unsigned address;
    char *name;

    struct backpatch *next;
};

struct parse_data {
    FILE *out;
    unsigned code_pos;
    int error_count;
    struct token *first_token;
    struct token *here;
};


char *str_dup(const char *source);
void str_trim(char *str);

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

void write_byte(struct parse_data *state, uint8_t value);
void write_short(struct parse_data *state, uint16_t value);
void write_long(struct parse_data *state, uint32_t value);

void print_location(struct token *token);
const char* type_name(enum token_type type);
int require_type(struct parse_data *state, enum token_type type);
int matches_type(struct parse_data *state, enum token_type type);
void skip_line(struct token **current);
void parse_error(struct parse_data *data, const char *err_msg);

#endif
