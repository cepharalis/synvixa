#pragma once

#include "lexer.h"
#include "file/file.h"

typedef struct language {
	char* name;
	char** extensions;
	size_t extension_count;
	token_type_vector token_types;
} language;

DEFINE_VECTOR(language)