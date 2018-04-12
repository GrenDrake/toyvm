#include <stdio.h>
#include <stdlib.h>

#include "toyvm.h"


unsigned char test_memory[] = {
    OP_LOADI,   0, 0, 10,
    OP_SAYNUM,  0, 0, 0,
    OP_LOADI,   1, 0, 1,
    OP_SUB,     0, 1, 0,
    OP_JUMPNZ,  0, 0xFF, 0xF0,

    OP_LOADWI,  0, 0, 0,
    OP_SAYNUM,  0, 0, 0,

    OP_LOADI,   4, 0, 4,
    OP_LOADWR,  5, 4, 0,
    OP_SAYNUM,  5, 0, 0,

    OP_EXIT,    0, 0, 0,
};


int main() {
    struct vmstate vm;

    vm_init_memory(&vm, sizeof(test_memory), test_memory);
    vm_run(&vm, 0);
    vm_free(&vm);

    return 0;
}
