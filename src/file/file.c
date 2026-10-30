#include "file.h"

#include <stdio.h>

file file_read(char* path) {
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
    		length--;
    		if (length > 0 && line[length - 1] == '\r') {
    			length--;
    		}
    		char_vector v;
    		char_vector_from_data(&v, line, length);
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
	if (line && length > 0) {
		if (length > 0 && line[length - 1] == '\r') {
			length--;
		}
		char_vector v;
		char_vector_from_data(&v, line, length);
		char_vector_vector_push(&f.lines, v);
	}
	free(line);
    fclose(fp);
    return f;
}

void file_write(const file* f) {
	FILE* fp = fopen(f->path, "w");
	if (fp == nullptr) {
		return;
	}
	for (size_t line = 0; line < f->lines.size; line++) {
		fwrite(f->lines.data[line].data, f->lines.data[line].size, 1, fp);
		if (line != f->lines.size - 1) {
			fwrite("\n", 1, 1, fp);
		}
	}
	fclose(fp);
}
