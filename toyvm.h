#ifndef TOYVM_H_4356346157
#define TOYVM_H_4356346157

struct vm_frame {
    int func_addr;
    int ret_addr;

    struct vm_frame *prev;
    struct vm_frame *next;
};

struct vmstate {
    int *stack;
    unsigned stack_size;
    int *stack_ptr;
    int *frame_ptr;
    unsigned char *ip;

    unsigned char *fixed_memory;
    unsigned memory_size;
};

int vm_init_memory(struct vmstate *vm, unsigned memory_size, unsigned char *memory);
int vm_run(struct vmstate *vm, unsigned start_address);
int vm_free(struct vmstate *vm);

int vm_read_byte(struct vmstate *vm, unsigned address);
int vm_read_short(struct vmstate *vm, unsigned address);
int vm_read_word(struct vmstate *vm, unsigned address);
void vm_store_word(struct vmstate *vm, unsigned address, unsigned value);
void vm_store_short(struct vmstate *vm, unsigned address, unsigned value);
void vm_store_byte(struct vmstate *vm, unsigned address, unsigned value);

#endif
