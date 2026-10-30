#pragma once

#include <regex.h>

#include "util/vector.h"

typedef enum token_group {
	NORMAL,
	KEYWORD,
	STRING,
	COMMENT,
	CONSTANT,
	PREPROCESSOR,
	ERROR
} token_group;

typedef struct token_type {
	char* name;
	regex_t pattern;
	token_group group;
} token_type;

DEFINE_VECTOR(token_type)

token_type token_type_new(char* name, const char* pattern, token_group group);

typedef struct token {
	size_t type;
	char* value;
	size_t line;
	size_t pos;
	size_t length;
} token;

DEFINE_VECTOR(token)

typedef struct lexer {
	token_type_vector* types;
	char_vector_vector* source;
	size_t line;
	size_t pos;
} lexer;

lexer lexer_new(token_type_vector* types, char_vector_vector* source);

bool lexer_has_next(const lexer* l);

token lexer_next(lexer* l);