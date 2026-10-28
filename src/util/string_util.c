#include "string_util.h"

#include <stddef.h>
#include <string.h>

bool string_ends_with(const char* str, const char* suffix) {
	if (!str || !suffix) {
		return false;
	}
	const size_t str_len = strlen(str);
	const size_t suffix_len = strlen(suffix);
	if (suffix_len > str_len) {
		return false;
	}
	return strcmp(str + str_len - suffix_len, suffix) == 0;
}
