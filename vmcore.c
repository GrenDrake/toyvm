#include <stdio.h>
#include <stdlib.h>

#include "toyvm.h"

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
    for (int i = 0; i < REGISTER_COUNT; ++i) {
        vm->reg[i] = 0;
    }
    vm->fixed_memory = memory_source;
    vm->memory_size = memory_size;
    return 1;
}

int vm_run(struct vmstate *vm, unsigned start_address) {
    if (start_address >= vm->memory_size) {
        return 0;
    }

    unsigned full_code, opcode, limmed, simmed, r1, r2, r3;
    int pc = start_address;
    while (1) {
        if (pc >= vm->memory_size) {
            fprintf(stderr,
                    "Tried to execute instruction at 0x%08X which is outside memory sized 0x%08X\n",
                    pc, vm->memory_size);
            return 0;
        }

        full_code = vm_read_word(vm->fixed_memory, pc);
        opcode = (full_code & 0xFF000000) >> 24;
        limmed = (full_code & 0x00FFFFFF);
        simmed = (full_code & 0x0000FFFF);
        r1     = (full_code & 0x00FF0000) >> 16;
        r2     = (full_code & 0x0000FF00) >> 8;
        r3     = (full_code & 0x000000FF);

        pc += 4;
        switch(opcode) {
            case OP_EXIT:
                return 1;
            case OP_LOADI:
                vm->reg[r1] = simmed;
                break;
            case OP_LOADWI:
                vm->reg[r1] = vm_read_word(vm->fixed_memory, simmed);
                break;
            case OP_LOADWR:
                vm->reg[r1] = vm_read_word(vm->fixed_memory, vm->reg[r2]);
                break;

            case OP_ADD:
                vm->reg[r3] = vm->reg[r1] + vm->reg[r2];
                break;
            case OP_SUB:
                vm->reg[r3] = vm->reg[r1] - vm->reg[r2];
                break;
            case OP_MUL:
                vm->reg[r3] = vm->reg[r1] * vm->reg[r2];
                break;
            case OP_DIV:
                vm->reg[r3] = vm->reg[r1] / vm->reg[r2];
                break;
            case OP_MOD:
                vm->reg[r3] = vm->reg[r1] % vm->reg[r2];
                break;

            case OP_SAYNUM:
                printf("number is %d\n", vm->reg[r1]);
                break;

            case OP_JUMP:
                pc = limmed;
                break;
            case OP_JUMPREL:
                if (limmed & 0x800000) limmed |= 0xFF000000;
                pc += (signed)limmed;
                break;
            case OP_JUMPZ:
                if (simmed & 0x8000) simmed |= 0xFFFF0000;
                if (vm->reg[r1] == 0) {
                    pc += (signed)simmed;
                }
                break;
            case OP_JUMPNZ:
                if (simmed & 0x8000) simmed |= 0xFFFF0000;
                if (vm->reg[r1] != 0) {
                    pc += (signed)simmed;
                }
                break;

            default:
                fprintf(stderr,
                        "Tried to execute unknown instruction 0x%X at address 0x%08X.\n",
                        opcode, pc - 4);
                return 0;
        }
    }

    return 1;
}

int vm_free(struct vmstate *vm) {
    return 1;
}
