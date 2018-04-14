#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble.h"
#include "opcode.h"

#define HEADER_SIZE 12

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

static void skip_line(struct token **current);
static void parse_error(struct token *where, const char *err_msg);
static int add_label(struct label_def **first_lbl, const char *name, int pos);
static void dump_labels(struct label_def *first);
static void free_labels(struct label_def *first);

struct mnemonic mnemonics[] = {
    {   op_exit,    "exit",     0 },
    
    {   op_stkdup,  "stkdup",   0 },

    {   op_pushb,   "pushb",    1 },
    {   op_pushs,   "pushs",    2 },
    {   op_pushw,   "pushw",    4 },
    {   op_readb,   "readb",    0 },
    {   op_reads,   "reads",    0 },
    {   op_readw,   "readw",    0 },

    {   op_add,     "add",      0 },
    {   op_sub,     "sub",      0 },
    {   op_mul,     "mul",      0 },
    {   op_div,     "div",      0 },
    {   op_mod,     "mod",      0 },

    {   op_saynum,  "saynum",   0 },
    {   op_saychar, "saychar",  0 },
    {   op_saystr,  "saystr",   0 },

    {   op_jump,    "jump",     0 },
    {   op_jumprel, "jumprel",  0 },
    {   op_jz,      "jz",       0 },
    {   op_jnz,     "jnz",      0 },
    {   op_bad,     NULL,       0 }
};


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

static struct label_def* get_label(struct label_def *first, const char *name) {
    struct label_def *current = first;
    
    while (current) {
        if (strcmp(name, current->name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
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

    FILE *out = fopen("output.bc", "wb+");
    if (!out) {
        printf("could not open output file\n");
        return 0;
    }

    // write empty header
    for (int i = 0; i < HEADER_SIZE; ++i) {
        fputc(0, out);
        ++code_pos;
    }

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

        if (strcmp(here->text, ".string") == 0) {
            here = here->next;
            if (here->type != tt_string) {
                parse_error(here, "expected string");
                skip_line(&here);
                continue;
            }
            
            printf("0x%08X string ~%s~\n", code_pos, here->text);
            int pos = 0;
            while (here->text[pos] != 0) {
                fputc(here->text[pos], out);
                ++pos;
            }
            fputc(0, out);
            code_pos += pos + 1;
            skip_line(&here);
            continue;
        }


        struct mnemonic *m = mnemonics;
        while (m->name && strcmp(m->name, here->text) != 0) {
            ++m;
        }
        if (m->name == NULL) {
            parse_error(here, "unknown mnemonic");
            skip_line(&here);
            continue;
        }

        printf("0x%08X %s/%d", code_pos, here->text, m->opcode);
        fputc(m->opcode, out);
        code_pos += 1;
        
        if (here->next && here->next->type != tt_eol) {
            if (m->operand_size == 0) {
                parse_error(here, "expected operand");
                skip_line(&here);
                continue;
            }
            
            struct label_def *label;
            struct token *operand = here->next;
            int op_value = 0;
            switch (operand->type) {
                case tt_integer:
                    op_value = operand->i;
                    break;
                case tt_identifier:
                    label = get_label(first_lbl, operand->text);
                    if (label) {
                        op_value = label->pos;
                    } else {
                        op_value = -1;
                    }
                    break;
                default:
                    parse_error(operand, "bad operand type");
                    skip_line(&here);
                    continue;
            }
            
            switch(m->operand_size) {
                case 1:
                    fputc(op_value, out);
                    break;
                case 2: {
                    short v = op_value;
                    fwrite(&v, 2, 1, out);
                    break; }
                case 4:
                    fwrite(&op_value, 4, 1, out);
                    break;
                default:
                    parse_error(operand, "(assembler) bad operand size");
            }
            printf("  op/%d: %d", m->operand_size, op_value);
            code_pos += m->operand_size;
        } else if (m->operand_size > 0) {
            parse_error(here, "unknown mnemonic");
            skip_line(&here);
            continue;
        }
        printf("\n");
        
        skip_line(&here);
    }

    // write header info
    fseek(out, 4, SEEK_SET);
    struct label_def *label = get_label(first_lbl, "start");
    if (label) {
        unsigned start_address = label->pos;
        fwrite(&start_address, 4, 1, out);
    } else {
        fprintf(stderr, "missing start label");
    }


    fclose(out);
    printf("\nLABELS\n");
    dump_labels(first_lbl);
    free_labels(first_lbl);
    return !has_errors;
}
