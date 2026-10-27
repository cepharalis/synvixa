#include "file.h"

#include <stdio.h>

file file_open(char* path) {
	file f;
	char_vector_vector_init(&f.lines, 0);
    f.path = path;
    FILE* fp = fopen(path, "r");
    if (fp == nullptr) {
		return f;
    }
	char* line = nullptr;
	size_t len = 0;
    while (getline(&line, &len, fp) != -1) {
        char_vector v;
        char_vector_from_data(&v, line, len);
    	char_vector_vector_push(&f.lines, v);
    }
    fclose(fp);
    return f;
}
