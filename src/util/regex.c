#include "regex.h"

#ifdef _WIN32
#include <string.h>

#include "slre/slre.h"
#endif

regex regex_compile(const char* pattern) {
	regex r;
#if defined(__linux__) || defined(__APPLE__)
	regcomp(&r.regex, pattern, REG_EXTENDED);
#elifdef _WIN32
	r.pattern = pattern;
#else
#error Unsupported platform
#endif
	return r;
}

regex_match regex_find(const regex* r, const char* str) {
	regex_match match;
	match.start = 0;
	match.end = 0;
#if defined(__linux__) || defined(__APPLE__)
	regmatch_t m;
	match.match = regexec(&r->regex, str, 1, &m, 0) == 0;
	if (match.match) {
		match.start = m.rm_so;
		match.end = m.rm_eo;
	}
#elifdef _WIN32
	struct slre_cap caps[8];
	const int found = slre_match(r->pattern, str, (int) strlen(str), caps, 8, 0);
	match.match = found > 0;
	match.start = 0;
	match.end = found;
#else
#error Unsupported platform
#endif
	return match;
}
