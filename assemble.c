#include <stdio.h>
#include <stdlib.h>

#include "assemble.h"


int main() {
    struct token_list *tokens = lex_file("source.txt");
    if (tokens == NULL) {
        printf("Errors occured.\n");
        return 1;
    }

//    dump_tokens(tokens);
    parse_tokens(tokens);
    free_tokens(tokens);
    return 0;
}
