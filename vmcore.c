#include <stdio.h>
#include <stdlib.h>

#include "toyvm.h"
#include "opcode.h"

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

unsigned vm_read_word(unsigned char *memory, unsigned address);




unsigned vm_read_word(unsigned char *memory, unsigned address) {
    unsigned word = 0;
    word |= memory[address]     << 24;
    word |= memory[address + 1] << 16;
    word |= memory[address + 2] << 8;
    word |= memory[address + 3];
    return word;
}




int vm_init_memory(struct vmstate *vm, unsigned memory_size, unsigned char *memory_source) {
    vm->fixed_memory = memory_source;
    vm->memory_size = memory_size;
    
    vm->stack_size = 512;
    vm->stack = malloc(vm->stack_size);
    vm->stack_ptr = vm->stack;

    return vm->stack != NULL;
}

int vm_run(struct vmstate *vm, unsigned start_address) {
    if (start_address >= vm->memory_size) {
        return 0;
    }

    unsigned opcode, operand;
    int pc = start_address;
    while (1) {
        if (pc >= vm->memory_size) {
            fprintf(stderr,
                    "Tried to execute instruction at 0x%08X which is outside memory sized 0x%08X\n",
                    pc, vm->memory_size);
            return 0;
        }

        opcode = vm->fixed_memory[pc++];
        switch(opcode) {
            case op_exit:
                return 1;

            case op_stkdup:
                MIN_STACK(vm, 1);
                vm_stk_push(vm, vm_stk_peek(vm, 1));
                break;

            case op_pushb:
                vm_stk_push(vm, vm->fixed_memory[pc++]);
                break;
            case op_pushs:
                operand = vm->fixed_memory[pc++];
                operand |= vm->fixed_memory[pc++] << 8;
                vm_stk_push(vm, operand);
                break;
            case op_pushw:
                operand = vm->fixed_memory[pc++];
                operand |= vm->fixed_memory[pc++] << 8;
                operand |= vm->fixed_memory[pc++] << 16;
                operand |= vm->fixed_memory[pc++] << 24;
                vm_stk_push(vm, operand);
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

            case op_saynum:
                MIN_STACK(vm, 1);
                printf("%d\n", vm_stk_pop(vm));
                break;

            case op_jnz:
                MIN_STACK(vm, 2);

                if (vm_stk_peek(vm, 2) != 0) {
                    pc = vm_stk_peek(vm, 1);
                }
                vm->stack_ptr -= 2;
                break;

            default:
                fprintf(stderr,
                        "Tried to execute unknown instruction 0x%X at address 0x%08X.\n",
                        opcode, pc-1);
                return 0;
        }
    }

    return 1;
}

int vm_free(struct vmstate *vm) {
    return 1;
}
