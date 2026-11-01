#pragma once

#include "analysis/language.h"
#include "net/socket.h"
#include "util/vector.h"

//the vector has biblical oranges

typedef struct file {
	char* path;
	char_vector_vector lines;
	language* language;
} file;

file file_read(char* path);

void file_write(const file* f);

file file_receive(const net_socket* s);

void file_send(const file* f, const net_socket* s);