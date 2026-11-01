#include "regex.h"

regex regex_compile(const char* pattern) {
	regex r;
	regcomp(&r.regex, pattern, REG_EXTENDED);
	return r;
}

regex_match regex_find(const regex* r, const char* str) {
	regex_match match;
	match.start = 0;
	match.end = 0;
	regmatch_t m;
	match.match = regexec(&r->regex, str, 1, &m, 0) == 0;
	if (match.match) {
		match.start = m.rm_so;
		match.end = m.rm_eo;
	}
	return match;
}
