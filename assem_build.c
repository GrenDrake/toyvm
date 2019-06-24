#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble.h"
#include "opcode.h"

static void add_patch(struct parse_data *state, unsigned pos, const char *name);
static int data_string(struct parse_data *state);
static int data_zeroes(struct parse_data *state);
static int data_bytes(struct parse_data *state, int width);

static int data_string(struct parse_data *state);
static int data_zeroes(struct parse_data *state);
static int data_bytes(struct parse_data *state, int width);


/* ************************************************************************* *
 * OPCODE DEFINITIONS                                                        *
 * ************************************************************************* */
struct mnemonic mnemonics[] = {
    {   op_exit,    "exit",     0 },

    {   op_stkdup,  "stkdup",   0 },

    {   op_pushb,   "pushb",    1 },
    {   op_pushs,   "pushs",    2 },
    {   op_pushw,   "pushw",    4 },
    {   op_readb,   "readb",    0 },
    {   op_reads,   "reads",    0 },
    {   op_readw,   "readw",    0 },
    {   op_storeb,  "storeb",   0 },
    {   op_stores,  "stores",   0 },
    {   op_storew,  "storew",   0 },

    {   op_add,     "add",      0 },
    {   op_sub,     "sub",      0 },
    {   op_mul,     "mul",      0 },
    {   op_div,     "div",      0 },
    {   op_mod,     "mod",      0 },
    {   op_inc,     "inc",      0 },
    {   op_dec,     "dec",      0 },

    {   op_gets,    "gets",     0 },
    {   op_saynum,  "saynum",   0 },
    {   op_saychar, "saychar",  0 },
    {   op_saystr,  "saystr",   0 },

    {   op_call,    "call",     0 },
    {   op_ret,     "ret",      0 },
    {   op_jump,    "jump",     0 },
    {   op_jumprel, "jumprel",  0 },
    {   op_jz,      "jz",       0 },
    {   op_jnz,     "jnz",      0 },
    {   op_bad,     NULL,       0 }
};


/* ************************************************************************* *
 * GENERAL UTILITY                                                           *
 * ************************************************************************* */
void add_patch(struct parse_data *state, unsigned pos, const char *name) {
    struct backpatch *patch = malloc(sizeof(struct backpatch));
    if (!patch) return;
    patch->address = pos;
    patch->name = str_dup(name);
    patch->next = state->patches;
    state->patches = patch;
}

void write_byte(struct parse_data *state, uint8_t value) {
    fputc(value, state->out);
    ++state->code_pos;
}
void write_short(struct parse_data *state, uint16_t value) {
    fwrite(&value, 2, 1, state->out);
    state->code_pos += 2;
}
void write_long(struct parse_data *state, uint32_t value) {
    fwrite(&value, 4, 1, state->out);
    state->code_pos += 4;
}


/* ************************************************************************* *
 * DIRECTIVE PROCESSING                                                      *
 * ************************************************************************* */
int data_bytes(struct parse_data *state, int width) {
    state->here = state->here->next;
#ifdef DEBUG
    printf("0x%08X data(%d)", *code_pos, width);
#endif

    while (state->here && state->here->type != tt_eol) {
        if (!require_type(state, tt_integer)) {
#ifdef DEBUG
            printf("\n");
#endif
            parse_error(state, "expected integer");
            return 0;
        }

#ifdef DEBUG
        printf(" %d", here->i);
#endif
        fwrite(&state->here->i, width, 1, state->out);
        state->code_pos += width;
        state->here = state->here->next;
    }
#ifdef DEBUG
    printf("\n");
#endif
    return 1;
}

int data_define(struct parse_data *state) {
    state->here = state->here->next;
    if (state->here->type != tt_identifier) {
        parse_error(state, "expected identifier");
        return 0;
    }

    const char *name = state->here->text;
    if (get_label(state, name)) {
        parse_error(state, "name already in use");
        return 0;
    }

    state->here = state->here->next;
    if (state->here->type != tt_integer) {
        parse_error(state, "expected integer");
        return 0;
    }

    if (!add_label(state, name, state->here->i)) {
        parse_error(state, "error creating constant");
        return 0;
    }
    skip_line(&state->here);
    return 1;
}

int data_export(struct parse_data *state) {
    uint32_t count = 0;
    long countpos = ftell(state->out);
    write_long(state, 0);

    state->here = state->here->next;
    while (!matches_type(state, tt_eol)) {
        if (require_type(state, tt_identifier)) {
            char buffer[20] = "";

            if (strlen(state->here->text) > 16) {
                strncpy(buffer, state->here->text, 16);
                parse_warn(state, "Label longer than 16 characters; export name truncated.");
            } else {
                strcpy(buffer, state->here->text);
            }
            fwrite(buffer, 16, 1, state->out);
            state->code_pos += 16;
            add_patch(state, ftell(state->out), state->here->text);
            write_long(state, 0);
            ++count;
        }
        state->here = state->here->next;
    }
    fseek(state->out, countpos, SEEK_SET);
    fwrite(&count, 4, 1, state->out);
    fseek(state->out, 0, SEEK_END);
    return 1;
}

int data_string(struct parse_data *state) {
    state->here = state->here->next;

    if (!require_type(state, tt_string)) {
        return 0;
    }

#ifdef DEBUG
    printf("0x%08X string ~%s~\n", *code_pos, here->text);
#endif
    for (int pos = 0; state->here->text[pos] != 0; ++pos) {
        write_byte(state, state->here->text[pos]);
    }
    write_byte(state, 0);
    skip_line(&state->here);
    return 1;
}

int data_mapdata(struct parse_data *state) {
    if (get_label(state, "mapdata")) {
        parse_error(state, "mapdata label already exists");
        return 0;
    } else if (!add_label(state, "mapdata", state->code_pos)) {
        parse_error(state, "could not create label for mapdata (already exists?)");
        return 0;
    }

    state->here = state->here->next;
    if (!require_type(state, tt_string)) {
        return 0;
    }
    struct map_data *data = map_reader(state->here->text);
    if (!data) {
        parse_error(state, "Failed to read mapdata.");
        return 0;
    } else {
        // write data
        write_short(state, data->width);
        write_short(state, data->height);
        struct map_line *line = data->data;
        while (line) {
            for (unsigned i = 0; i < data->width; ++i) {
                write_byte(state, state->tile_mapping[(int)line->data[i]]);
            }
            line = line->next;
        }
        free_mapdata(data);
    }
    skip_line(&state->here);
    return 1;
}

int data_tileinfo(struct parse_data *state) {
    state->here = state->here->next;
    if (!require_type(state, tt_integer)) {
        return 0;
    }
    int mapChar = state->here->i;

    state->here = state->here->next;
    if (!require_type(state, tt_integer)) {
        parse_error(state, "Expected map tile value.");
        return 0;
    }
    int mapTile = state->here->i;
    state->tile_mapping[mapChar] = mapTile;
    skip_line(&state->here);
    return 1;
}

int data_zeroes(struct parse_data *state) {
    state->here = state->here->next;
    if (!require_type(state, tt_integer)) {
        return 0;
    }

#ifdef DEBUG
    printf("0x%08X zeroes (%d)\n", *code_pos, here->i);
#endif
    for (int i = 0; i < state->here->i; ++i) {
        write_byte(state, 0);
    }
    skip_line(&state->here);
    return 1;
}



/* ************************************************************************* *
 * CORE PARSING ROUTINE                                                      *
 * ************************************************************************* */
int parse_tokens(struct token_list *list, const char *output_filename) {
    struct parse_data state = { NULL };
    int done_initial = 0;

    state.out = fopen(output_filename, "wb+");
    if (!state.out) {
        fprintf(stderr, "FATAL: could not open output file\n");
        return 1;
    }

    // write empty header
    for (int i = 0; i < HEADER_SIZE; ++i) {
        write_byte(&state, 0);
    }

    state.first_token = state.here = list->first;
    while (state.here) {
        if (state.here->type == tt_eol) {
            state.here = state.here->next;
            continue;
        }

        if (state.here->type != tt_identifier) {
            parse_error(&state, "expected identifier");
            continue;
        }

        if (strcmp(state.here->text, ".export") == 0) {
            if (done_initial) {
                parse_error(&state, ".export must precede other statements");
                continue;
            }
            data_export(&state);
            continue;
        }

        if (strcmp(state.here->text, ".tileinfo") == 0) {
            if (done_initial) {
                parse_error(&state, ".tileinfo must precede other statements except exports");
                continue;
            }
            data_tileinfo(&state);
            continue;
        }

        if (strcmp(state.here->text, ".mapdata") == 0) {
            if (done_initial) {
                parse_error(&state, ".mapdata must precede other statements except exports");
                continue;
            }
            data_mapdata(&state);
            continue;
        }

        done_initial = 1;

        if (state.here->next && state.here->next->type == tt_colon) {
            if (get_label(&state, state.here->text)) {
                parse_error(&state, "label already defined");
            } else if (!add_label(&state, state.here->text, state.code_pos)) {
                parse_error(&state, "could not create label");
            } else {
                state.here = state.here->next->next;
            }
            continue;
        }

        if (strcmp(state.here->text, ".string") == 0) {
            data_string(&state);
            continue;
        }

        if (strcmp(state.here->text, ".byte") == 0) {
            data_bytes(&state, 1);
            continue;
        }
        if (strcmp(state.here->text, ".short") == 0) {
            data_bytes(&state, 2);
            continue;
        }
        if (strcmp(state.here->text, ".word") == 0) {
            data_bytes(&state, 4);
            continue;
        }
        if (strcmp(state.here->text, ".zero") == 0) {
            data_zeroes(&state);
            continue;
        }

        if (strcmp(state.here->text, ".define") == 0) {
            data_define(&state);
            continue;
        }

        if (strcmp(state.here->text, ".include") == 0) {
            state.here = state.here->next;
            if (state.here->type != tt_string) {
                parse_error(&state, "expected string");
                continue;
            }

            struct token_list *new_tokens = lex_file(state.here->text);
            if (new_tokens == NULL) {
                fprintf(stderr, "Failed to lex included file %s.\n", state.here->text);
                return 1 + state.error_count;
            }

            new_tokens->last->next = state.here->next;
            state.here->next = new_tokens->first;
            free(new_tokens);

            skip_line(&state.here);
            continue;
        }


        struct mnemonic *m = mnemonics;
        while (m->name && strcmp(m->name, state.here->text) != 0) {
            ++m;
        }
        if (m->name == NULL) {
            parse_error(&state, "unknown mnemonic");
            continue;
        }

#ifdef DEBUG
        printf("0x%08X %s/%d", code_pos, here->text, m->opcode);
#endif
        write_byte(&state, m->opcode);

        if (state.here->next && state.here->next->type != tt_eol) {
            if (m->operand_size == 0) {
                parse_error(&state, "expected operand");
                continue;
            }

            struct label_def *label;
            struct token *operand = state.here->next;
            int op_value = 0;
            switch (operand->type) {
                case tt_integer:
                    op_value = operand->i;
                    break;
                case tt_identifier:
                    label = get_label(&state, operand->text);
                    if (label) {
                        op_value = label->pos;
                    } else {
                        op_value = -1;
                        add_patch(&state, ftell(state.out), operand->text);
                    }
                    break;
                default:
                    parse_error(&state, "bad operand type");
                    continue;
            }

            switch(m->operand_size) {
                case 1:
                    fputc(op_value, state.out);
                    break;
                case 2: {
                    short v = op_value;
                    fwrite(&v, 2, 1, state.out);
                    break; }
                case 4:
                    fwrite(&op_value, 4, 1, state.out);
                    break;
                default:
                    parse_error(&state, "(assembler) bad operand size");
            }
#ifdef DEBUG
            printf("  op/%d: %d", m->operand_size, op_value);
#endif
            state.code_pos += m->operand_size;
        } else if (m->operand_size > 0) {
            parse_error(&state, "unknown mnemonic");
            continue;
        }
#ifdef DEBUG
        printf("\n");
#endif

        skip_line(&state.here);
    }

    // write header info
    fseek(state.out, 0, SEEK_SET);
    fwrite("TVM", 4, 1, state.out);

    // update backpatches
    struct backpatch *patch = state.patches;
    while (patch) {
        struct label_def *label = get_label(&state, patch->name);
        if (!label) {
            fprintf(stderr, "Undefined symbol %s.\n", patch->name);
            ++state.error_count;
        } else {
            fseek(state.out, patch->address, SEEK_SET);
            uint32_t v = label->pos;
            fwrite(&v, 4, 1, state.out);
        }
        struct backpatch *next = patch->next;
        free(patch->name);
        free(patch);
        patch = next;
    }

    // all done writing file
    fclose(state.out);
    dump_labels(&state);
    free_labels(&state);
    return state.error_count;
}
