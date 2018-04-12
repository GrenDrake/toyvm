#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble.h"

struct label_def {
    char *name;
    int pos;
    struct label_def *next;
};

static void skip_line(struct token **current);
static void parse_error(struct token *where, const char *err_msg);
static int add_label(struct label_def **first_lbl, const char *name, int pos);
static void dump_labels(struct label_def *first);
static void free_labels(struct label_def *first);


static void skip_line(struct token **current) {
    struct token *here = *current;

    while (here && here->type != tt_eol) {
        here = here->next;
    }

    if (here) {
        *current = here->next;
    } else {
        *current = NULL;
    }
}

static void parse_error(struct token *where, const char *err_msg) {
    printf("%s:%d:%d %s\n",
           where->source_file,
           where->line,
           where->column,
           err_msg);
}

static int add_label(struct label_def **first_lbl, const char *name, int pos) {
    struct label_def *cur = *first_lbl;

    struct label_def *new_lbl = malloc(sizeof(struct label_def));
    if (!new_lbl) {
        return 0;
    }
    new_lbl->name = str_dup(name);
    new_lbl->pos = pos;

    if (cur == NULL) {
        new_lbl->next = NULL;
    } else {
        new_lbl->next = cur;
    }
    *first_lbl = new_lbl;
    return 1;
}

static void dump_labels(struct label_def *first) {
    struct label_def *cur = first;
    while (cur) {
        printf("0x%08X  %s\n", cur->pos, cur->name);
        cur = cur->next;
    }
}

static void free_labels(struct label_def *first) {
    struct label_def *cur = first;
    while (cur) {
        struct label_def *next = cur->next;
        free(cur->name);
        free(cur);
        cur = next;
    }
}

int parse_tokens(struct token_list *list) {
    int has_errors = 0;
    int code_pos = 0;
    int done_initial = 0;
    struct label_def *first_lbl = NULL;

    struct token *here = list->first;
    while (here) {
        if (here->type == tt_eol) {
            here = here->next;
            continue;
        }

        if (here->type != tt_identifier) {
            parse_error(here, "expected identifier");
            has_errors += 1;
            skip_line(&here);
            continue;
        }

        if (strcmp(here->text, ".export") == 0) {
            if (done_initial) {
                parse_error(here, ".export must precede other statements");
                skip_line(&here);
                continue;
            } else {
                skip_line(&here);
                continue;
            }
        }
        
        done_initial = 1;

        if (here->next && here->next->type == tt_colon) {
            add_label(&first_lbl, here->text, code_pos);
            here = here->next->next;
            continue;
        }

        printf("tok %s\n", here->text);
        code_pos += 4;
        skip_line(&here);
    }

    dump_labels(first_lbl);
    free_labels(first_lbl);
    return !has_errors;
}
