#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble.h"


void free_mapdata(struct map_data *data) {
    struct map_line *line = data->data;
    while (line) {
        struct map_line *next = line->next;
        free(line);
        line = next;
    }
    free(data);
}

struct map_data* map_reader(const char *source_file) {
    FILE *in = fopen(source_file, "rt");
    if (!in) {
        fprintf(stderr, "%s: could not open file\n", source_file);
        return NULL;
    }
    struct map_data *data = malloc(sizeof(struct map_data));
    if (!data) {
        fprintf(stderr, "%s: memory allocation error\n", source_file);
        fclose(in);
        return NULL;
    }
    data->data = NULL;

    char inbuf[501];
    fgets(inbuf, 500, in);
    str_trim(inbuf);
    struct map_line *last;
    data->width = strlen(inbuf);
    data->data = malloc(sizeof(struct map_line));
    data->data->data = str_dup(inbuf);
    last = data->data;
    data->height = 1;
    while (1) {
        fgets(inbuf, 500, in);
        if (feof(in)) break;

        str_trim(inbuf);
        if (strlen(inbuf) != data->width) {
            fprintf(stderr, "%s: unexpected width of map line\n", source_file);
            free_mapdata(data);
            fclose(in);
            return NULL;
        }
        last->next = malloc(sizeof(struct map_line));
        last->next->data = str_dup(inbuf);
        last = last->next;
        ++data->height;
    }

    fclose(in);
    return data;
}