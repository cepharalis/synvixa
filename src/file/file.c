#include "file.h"

#include <stdint.h>
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

file file_receive(const net_socket* s) {
	uint32_t header[2];
	int received = 0;
	while ((received += socket_read(s, (char*) &header + received, 8 - received)) < 8) {}
	const uint32_t lines = header[0];
	const uint32_t path_length = header[1];
	char* path = malloc(path_length + 1);
	path[path_length] = '\0';
	received = 0;
	while ((received += socket_read(s, path + received, (int) path_length - received)) < path_length) {}
	file f;
	f.path = path;
	f.language = nullptr;
	char_vector_vector_init(&f.lines, lines);
	for (uint32_t line = 0; line < lines; line++) {
		uint32_t length = 0;
		received = 0;
		while ((received += socket_read(s, (char*) &length + received, 4 - received)) < 4) {}
		char_vector line_buffer;
		char_vector_init(&line_buffer, length);
		received = 0;
		while ((received += socket_read(s, line_buffer.data + received, (int) length - received)) < length) {}
		line_buffer.size = length;
		char_vector_vector_push(&f.lines, line_buffer);
	}
	return f;
}

void file_send(const file* f, const net_socket* s) {
	const uint32_t lines = f->lines.size;
	socket_write(s, &lines, 4);
	const uint32_t path_length = strlen(f->path);
	socket_write(s, &path_length, 4);
	socket_write(s, f->path, (int) path_length);
	for (uint32_t line = 0; line < f->lines.size; line++) {
		const uint32_t length = f->lines.data[line].size;
		socket_write(s, &length, 4);
		socket_write(s, f->lines.data[line].data, (int) length);
	}
}
