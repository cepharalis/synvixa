#pragma once

#include "util/vector.h"

DEFINE_VECTOR(char_vector)

typedef struct file {
	char* path;
	char_vector_vector lines;
} file;