#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble.h"


int add_label(struct parse_data *state, const char *name, int pos) {
    struct label_def *existing = get_label(state, name);
    if (existing) {
        return 0;
    }

    struct label_def *new_lbl = malloc(sizeof(struct label_def));
    if (!new_lbl) {
        return 0;
    }
    new_lbl->name = str_dup(name);
    new_lbl->pos = pos;
    new_lbl->next = state->first_label;
    state->first_label = new_lbl;
    return 1;
}

struct label_def* get_label(struct parse_data *state, const char *name) {
    struct label_def *current = state->first_label;

    while (current) {
        if (strcmp(name, current->name) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void dump_labels(struct parse_data *state) {
    FILE *out = fopen("labels.txt", "wt");
    if (!out) {
        fprintf(stderr, "Could not create labels.txt\n");
        return;
    }

    struct label_def *cur = state->first_label;
    while (cur) {
        fprintf(out, "0x%08X  %s\n", cur->pos, cur->name);
        cur = cur->next;
    }
    fclose(out);
}

void free_labels(struct parse_data *state) {
    struct label_def *cur = state->first_label;
    while (cur) {
        struct label_def *next = cur->next;
        free(cur->name);
        free(cur);
        cur = next;
    }
}
