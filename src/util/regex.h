#pragma once

#include <stddef.h>

#if defined(__linux__) || defined(__APPLE__)
#include <regex.h>
#elifndef _WIN32
#error Unsupported platform
#endif

typedef struct regex {
#if defined(__linux__) || defined(__APPLE__)
	regex_t regex;
#elifdef _WIN32
	const char* pattern;
#else
#error Unsupported platform
#endif
} regex;

typedef struct regex_match {
	bool match;
	size_t start;
	size_t end;
} regex_match;

regex regex_compile(const char* pattern);

regex_match regex_find(const regex* r, const char* str);