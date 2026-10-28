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
	size_t line_length = 0;
    while ((line_length = getline(&line, &len, fp)) != -1) {
        char_vector v;
    	if (line_length > 0 && line[line_length - 1] == '\n') {
			char_vector_from_data(&v, line, line_length - 1);
    	}
    	else {
    		char_vector_from_data(&v, line, line_length);
    	}
    	char_vector_vector_push(&f.lines, v);
    }
	free(line);
    fclose(fp);
    return f;
}
