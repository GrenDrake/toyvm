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

    int map_addr = vm_get_export(&vm, "mapdata");
    int w = vm_read_short(&vm, map_addr);
    int h = vm_read_short(&vm, map_addr + 2);
    printf("Map Size: %dx%d (%d bytes)\nMap Data Start: 0x%X\n\n", w, h, w*h, map_addr + 4);

    int start_addr = vm_get_export(&vm, "start");
    int run_failed = 0;
    if (start_addr < 0) {
        fprintf(stderr, "Could not find program start address.\n");
        run_failed = 1;
    } else {
        if (!vm_run(&vm, start_addr)) {
            fprintf(stderr, "vm error occured.\n");
            run_failed = 1;
        }
    }
    vm_free(&vm);

    free(memory);
    return !run_failed;
}
