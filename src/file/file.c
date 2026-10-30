#include "file.h"

#include <stdio.h>

file file_open(char* path) {
	file f;
	char_vector_vector_init(&f.lines, 0);
    f.path = path;
	f.language = nullptr;
    FILE* fp = fopen(path, "r");
    if (fp == nullptr) {
		return f;
    }
	size_t length = 0;
	size_t capacity = 256;
	char* line = malloc(capacity);
    while (fgets(line + length, (int) (capacity - length), fp)) {
    	length += strlen(line + length);
    	if (length > 0  && line[length - 1] == '\n') {
    		char_vector v;
    		char_vector_from_data(&v, line, length - 1);
    		char_vector_vector_push(&f.lines, v);
    		length = 0;
    	}
    	else {
    		capacity *= 2;
    		char* l = realloc(line, capacity);
    		if (!l) {
    			free(line);
    			break;
    		}
    		line = l;
    	}
    }
	free(line);
    fclose(fp);
    return f;
}
