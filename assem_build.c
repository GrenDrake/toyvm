#include <stdio.h>
#include <string.h>

#include "assemble.h"

struct label_def {
    char *name;
    int pos;
    struct label_def *next;
};

static void skip_line(struct token **current);
static void parse_error(struct token *where, const char *err_msg);

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

int parse_tokens(struct token_list *list) {
    int has_errors = 0;
    int code_pos = 0;
    int done_initial = 0;

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
            printf("LBL %s @ %d\n", here->text, code_pos);
            here = here->next->next;
            continue;
        }

        printf("tok %s\n", here->text);
        code_pos += 4;
        skip_line(&here);
    }

    return !has_errors;
}
