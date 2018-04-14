#include <stdio.h>
#include <stdlib.h>

#include "toyvm.h"



int main() {
    struct vmstate vm;
    
    FILE *in = fopen("output.bc", "rb");
    if (!in) {
        fprintf(stderr, "could not open vm image.\n");
        return 1;
    }
    fseek(in, 0, SEEK_END);
    unsigned filesize = ftell(in);
    rewind(in);

    unsigned char *memory = malloc(filesize);
    if (!memory) {
        fclose(in);
        fprintf(stderr, "memory allocation failed.\n");
        return 1;
    }

    fread(memory, filesize, 1, in);
    fclose(in);

    vm_init_memory(&vm, filesize, memory);
    if (!vm_run(&vm, 0)) {
        fprintf(stderr, "vm error occured.\n");
    }
    vm_free(&vm);

    free(memory);
    return 0;
}
