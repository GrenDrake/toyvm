#include <stdlib.h>
#include <string.h>

char *str_dup(const char *source) {
    size_t length = strlen(source);
    char *new_str = malloc(length + 1);
    if (new_str == NULL) return NULL;
    strcpy(new_str, source);
    return new_str;
}
