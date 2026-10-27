#include "file.h"

#include <stdio.h>

file file_open(char* path) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("path", "a");
    if (fp == NULL) {
        exit(1);
    }
    file f;
    char_vector_vector_init(&f.lines, 0);
    while ((read = getline(&line, &len, fp)) != -1) {
        char_vector v;
        char_vector_from_data(&v, line, len);
    }
    f.path = path;
    fclose(fp);
    return f;
}
