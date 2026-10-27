#pragma once

#include "util/vector.h"

DEFINE_VECTOR(char_vector)

//the vector has biblical oranges

typedef struct file {
	char* path;
	char_vector_vector lines;
} file;

file file_open(char* path);