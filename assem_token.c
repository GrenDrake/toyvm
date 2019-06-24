#include <stdio.h>

#include "assemble.h"

void print_location(struct token *token) {
    fprintf(stderr, "%s:%d:%d ",
           token->source_file,
           token->line,
           token->column);
}
const char* type_name(enum token_type type) {
    switch(type) {
        case tt_bad:        return "bad token";
        case tt_colon:      return "colon";
        case tt_eol:        return "end-of-line";
        case tt_identifier: return "identifier";
        case tt_integer:    return "integer";
        case tt_string:     return "string";
        default:            return "unknown token type";
    }
}

int require_type(struct parse_data *state, enum token_type type) {
    if (!state || !state->here) return 0;
    if (state->here->type == type) return 1;
    ++state->error_count;
    print_location(state->here);
    fprintf(stderr, "Expected %s, but found %s.\n",
                    type_name(type),
                    type_name(state->here->type));

    while (state->here && state->here->type != tt_eol) {
        state->here = state->here->next;
    }
    return 0;
}

int matches_type(struct parse_data *state, enum token_type type) {
    if (!state || !state->here) return 0;
    if (state->here->type == type) return 1;
    return 0;
}

void skip_line(struct token **current) {
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

void parse_warn(struct parse_data *data, const char *warn_msg) {
    ++data->error_count;
    print_location(data->here);
    printf("(warning) %s\n", warn_msg);
}

void parse_error(struct parse_data *data, const char *err_msg) {
    ++data->error_count;
    print_location(data->here);
    printf("%s\n", err_msg);

    while (data->here && data->here->type != tt_eol) {
        data->here = data->here->next;
    }
}