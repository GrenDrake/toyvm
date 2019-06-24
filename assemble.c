#include <stdio.h>
#include <stdlib.h>

#include "assemble.h"


int main(int argc, char *argv[]) {
    const char *infile  = "source.a";
    const char *outfile = "output.bc";

    if (argc >= 2) {
        infile = argv[1];
    }
    if (argc >= 3) {
        outfile = argv[2];
    }

    struct token_list *tokens = lex_file(infile);
    if (tokens == NULL) {
        printf("Errors occured.\n");
        return 1;
    }

//    dump_tokens(tokens);
    int error_count = parse_tokens(tokens, outfile);
    if (error_count > 0) {
        fprintf(stderr, "Found %d errors.\n", error_count);
    }
    free_tokens(tokens);
    return 0;
}
