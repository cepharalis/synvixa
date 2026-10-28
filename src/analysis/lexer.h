#pragma once

#include <regex.h>

#include "file/file.h"

typedef struct token_type {
	unsigned int type;
	regex_t pattern;
} token_type;

DEFINE_VECTOR(token_type)

typedef struct token {
	size_t type;
	char* value;
	size_t line;
	size_t pos;
} token;

typedef struct lexer {
	token_type_vector* types;
	char_vector_vector* source;
	size_t line;
	size_t pos;
} lexer;

lexer lexer_new(token_type_vector* types, char_vector_vector* source);

token lexer_next(lexer* l);