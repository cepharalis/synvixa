#pragma once

#include "pcre2posix.h"

typedef struct regex {
	regex_t regex;
} regex;

typedef struct regex_match {
	bool match;
	size_t start;
	size_t end;
} regex_match;

regex regex_compile(const char* pattern);

regex_match regex_find(const regex* r, const char* str);