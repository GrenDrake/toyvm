#ifndef TOYVM_H_4356346157
#define TOYVM_H_4356346157

#define OP_EXIT     0
#define OP_LOADI    1
#define OP_ADD      2
#define OP_SAYNUM   3
#define OP_JUMP     4
#define OP_JUMPZ    5
#define OP_JUMPNZ   6
#define OP_SUB      7
#define OP_JUMPREL  8
#define OP_MUL      9
#define OP_DIV      10
#define OP_MOD      11
#define OP_LOADWI   12
#define OP_LOADWR   13


#define REGISTER_COUNT 16

struct vmstate {
    int reg[REGISTER_COUNT];
    unsigned char *fixed_memory;
    unsigned memory_size;
};

int vm_init_memory(struct vmstate *vm, unsigned memory_size, unsigned char *memory);
int vm_run(struct vmstate *vm, unsigned start_address);
int vm_free(struct vmstate *vm);

#endif
