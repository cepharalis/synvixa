#pragma once

#include "analysis/language.h"
#include "util/vector.h"

//the vector has biblical oranges

typedef struct file {
	char* path;
	char_vector_vector lines;
	language* language;
} file;

file file_open(char* path);