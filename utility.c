#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *str_dup(const char *source) {
    size_t length = strlen(source);
    char *new_str = malloc(length + 1);
    if (new_str == NULL) return NULL;
    strcpy(new_str, source);
    return new_str;
}

void str_trim(char *str) {
    if (!str) return;
    if (str[0] == 0) return;

    long initial_length = strlen(str);
    long len = initial_length;
    --len;
    while (len > 0 && isspace(str[len])) {
        str[len] = 0;
        --len;
    }

    len = 0;
    while (str[len] != 0 && isspace(str[len])) ++len;
    memmove(str, &str[len], initial_length - len);
}
