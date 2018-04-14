#ifndef TOYVM_H_4356346157
#define TOYVM_H_4356346157

struct vmstate {
    int *stack;
    unsigned stack_size;
    int *stack_ptr;

    unsigned char *fixed_memory;
    unsigned memory_size;
};

int vm_init_memory(struct vmstate *vm, unsigned memory_size, unsigned char *memory);
int vm_run(struct vmstate *vm, unsigned start_address);
int vm_free(struct vmstate *vm);

#endif
