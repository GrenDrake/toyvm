#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toyvm.h"
#include "opcode.h"

#define EXPORT_COUNT_POS    12
#define EXPORT_FIRST        16
#define EXPORT_NAME_SIZE    16
#define EXPORT_SIZE         20

static inline void vm_stk_push(struct vmstate *vm, int value) {
    *vm->stack_ptr = value;
    ++vm->stack_ptr;
}
static inline int vm_stk_size(struct vmstate *vm) {
    return vm->stack_ptr - vm->stack;
}
static inline int vm_stk_peek(struct vmstate *vm, int pos) {
    return *(vm->stack_ptr - pos);
}
static inline int vm_stk_pop(struct vmstate *vm) {
    --vm->stack_ptr;
    return *vm->stack_ptr;
}
static inline void vm_stk_set(struct vmstate *vm, int pos, int val) {
    *(vm->stack_ptr - pos) = val;
}

#define MIN_STACK(vm, min_size) \
    if (vm_stk_size(vm) < min_size) { \
        fprintf(stderr, "stack underflow\n"); \
        return 0; \
    }


int vm_init_memory(struct vmstate *vm, unsigned memory_size, unsigned char *memory_source) {
    vm->fixed_memory = memory_source;
    vm->memory_size = memory_size;

    vm->frame_ptr = NULL;
    vm->stack_size = 512;
    vm->stack = malloc(vm->stack_size);
    vm->stack_ptr = vm->stack;
    return vm->stack != NULL;
}

int vm_get_export(struct vmstate *vm, const char *name) {
    int export_count = vm_read_word(vm, EXPORT_COUNT_POS);
    for (int i = 0; i < export_count; ++i) {
        int pos = EXPORT_FIRST + i * EXPORT_SIZE;
        char export_name[20] = { 0 };
        for (int j = 0; j < EXPORT_NAME_SIZE; ++j, ++pos) {
            export_name[j] = vm->fixed_memory[pos];
        }
        if (strcmp(export_name, name) == 0) {
            return vm_read_word(vm, pos);
        }
    }
    return -1;
}

int vm_run(struct vmstate *vm, unsigned start_address) {
    if (start_address >= vm->memory_size) {
        return 0;
    }

    unsigned opcode, operand, operand2;
    unsigned char *pc = &vm->fixed_memory[start_address];
    vm->frame_ptr = vm->stack_ptr;
    while (1) {
        if (pc >= &vm->fixed_memory[vm->memory_size]) {
            fprintf(stderr,
                    "Tried to execute instruction at 0x%08lX which is outside memory sized 0x%08X\n",
                    pc - vm->fixed_memory, vm->memory_size);
            return 0;
        }

        opcode = *pc++;
        switch(opcode) {
            case op_exit:
                return 1;

            case op_stkdup:
                MIN_STACK(vm, 1);
                vm_stk_push(vm, vm_stk_peek(vm, 1));
                break;

            case op_pushb:
                vm_stk_push(vm, *pc++);
                break;
            case op_pushs:
                operand = *pc++;
                operand |= (*pc++) << 8;
                vm_stk_push(vm, operand);
                break;
            case op_pushw:
                operand = *pc++;
                operand |= (*pc++) << 8;
                operand |= (*pc++) << 16;
                operand |= (*pc++) << 24;
                vm_stk_push(vm, operand);
                break;
            case op_readb:
                MIN_STACK(vm, 1);
                vm_stk_push(vm, vm_read_byte(vm, vm_stk_pop(vm)));
                break;
            case op_reads:
                MIN_STACK(vm, 1);
                vm_stk_push(vm, vm_read_short(vm, vm_stk_pop(vm)));
                break;
            case op_readw:
                MIN_STACK(vm, 1);
                vm_stk_push(vm, vm_read_word(vm, vm_stk_pop(vm)));
                break;
            case op_storeb:
                MIN_STACK(vm, 2);
                operand = vm_stk_pop(vm);
                vm_store_byte(vm, operand, vm_stk_pop(vm));
                break;
            case op_stores:
                MIN_STACK(vm, 2);
                operand = vm_stk_pop(vm);
                vm_store_short(vm, operand, vm_stk_pop(vm));
                break;
            case op_storew:
                MIN_STACK(vm, 2);
                operand = vm_stk_pop(vm);
                vm_store_word(vm, operand, vm_stk_pop(vm));
                break;

            case op_add:
                MIN_STACK(vm, 2);
                vm_stk_set(vm, 2, vm_stk_peek(vm, 2) + vm_stk_peek(vm, 1));
                vm_stk_pop(vm);
                break;
            case op_sub:
                MIN_STACK(vm, 2);
                vm_stk_set(vm, 2, vm_stk_peek(vm, 2) - vm_stk_peek(vm, 1));
                vm_stk_pop(vm);
                break;
            case op_mul:
                MIN_STACK(vm, 2);
                vm_stk_set(vm, 2, vm_stk_peek(vm, 2) * vm_stk_peek(vm, 1));
                vm_stk_pop(vm);
                break;
            case op_div:
                MIN_STACK(vm, 2);
                vm_stk_set(vm, 2, vm_stk_peek(vm, 2) / vm_stk_peek(vm, 1));
                vm_stk_pop(vm);
                break;
            case op_mod:
                MIN_STACK(vm, 2);
                vm_stk_set(vm, 2, vm_stk_peek(vm, 2) % vm_stk_peek(vm, 1));
                vm_stk_pop(vm);
                break;
            case op_inc:
                MIN_STACK(vm, 1);
                vm_stk_set(vm, 1, vm_stk_peek(vm, 1) + 1);
                break;
            case op_dec:
                MIN_STACK(vm, 1);
                vm_stk_set(vm, 1, vm_stk_peek(vm, 1) - 1);
                break;

            case op_gets:
                operand = vm_stk_pop(vm);
                fgets((char*)&vm->fixed_memory[operand + 1], vm_stk_pop(vm), stdin);
                operand2 = strlen((char*)&vm->fixed_memory[operand + 1]);
                vm->fixed_memory[operand] = operand2;
                vm->fixed_memory[operand + operand2] = 0;
                break;
            case op_saynum:
                MIN_STACK(vm, 1);
                printf("%d", vm_stk_pop(vm));
                break;
            case op_saychar:
                MIN_STACK(vm, 1);
                printf("%c", vm_stk_pop(vm));
                break;
            case op_saystr:
                MIN_STACK(vm, 1);
                printf("%s", &vm->fixed_memory[vm_stk_pop(vm)]);
                break;

            case op_call: {
                MIN_STACK(vm, 1);
                int target = vm_stk_pop(vm);
                vm_stk_push(vm, pc - vm->fixed_memory);
                vm_stk_push(vm, vm->frame_ptr - vm->stack);
                pc = &vm->fixed_memory[target];
                vm->frame_ptr = vm->stack_ptr;
                break; }
            case op_ret: {
                MIN_STACK(vm, 1);
                int retval = vm_stk_pop(vm);
                vm->stack_ptr = vm->frame_ptr;
                vm->frame_ptr = vm->stack + vm_stk_pop(vm);
                pc = vm->fixed_memory + vm_stk_pop(vm);
                vm_stk_push(vm, retval);
                break; }

            case op_jnz:
                MIN_STACK(vm, 2);

                if (vm_stk_peek(vm, 2) != 0) {
                    pc = &vm->fixed_memory[vm_stk_peek(vm, 1)];
                }
                vm->stack_ptr -= 2;
                break;

            default:
                fprintf(stderr,
                        "Tried to execute unknown instruction 0x%X at address 0x%08lX.\n",
                        opcode, pc - vm->fixed_memory);
                return 0;
        }
    }

    return 1;
}

int vm_free(struct vmstate *vm) {
    return 1;
}

int vm_read_byte(struct vmstate *vm, unsigned address) {
    return vm->fixed_memory[address];
}

int vm_read_short(struct vmstate *vm, unsigned address) {
    unsigned word = 0;
    word |= vm->fixed_memory[address];
    word |= vm->fixed_memory[address + 1] << 8;
    return word;
}

int vm_read_word(struct vmstate *vm, unsigned address) {
    unsigned word = 0;
    word |= vm->fixed_memory[address];
    word |= vm->fixed_memory[address + 1] << 8;
    word |= vm->fixed_memory[address + 2] << 16;
    word |= vm->fixed_memory[address + 3] << 24;
    return word;
}

void vm_store_byte(struct vmstate *vm, unsigned address, unsigned value) {
    vm->fixed_memory[address] = value & 0xFF;
}

void vm_store_short(struct vmstate *vm, unsigned address, unsigned value) {
    vm->fixed_memory[address]     = value & 0xFF;
    vm->fixed_memory[address + 1] = (value >> 8) & 0xFF;
}

void vm_store_word(struct vmstate *vm, unsigned address, unsigned value) {
    vm->fixed_memory[address]     = value & 0xFF;
    vm->fixed_memory[address + 1] = (value >> 8)  & 0xFF;
    vm->fixed_memory[address + 2] = (value >> 16) & 0xFF;
    vm->fixed_memory[address + 3] = (value >> 24) & 0xFF;
}

