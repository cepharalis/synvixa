#include "lexer.h"

token_type token_type_new(char* name, const char* pattern) {
	regex_t regex;
	char* p = malloc(strlen(pattern) + 2);
	p[0] = '^';
	strcpy(p + 1, pattern);
	regcomp(&regex, p, REG_EXTENDED);
	return (token_type){.name = name, .pattern = regex};
}

lexer lexer_new(token_type_vector* types, char_vector_vector* source) {
	lexer l;
	l.types = types;
	l.source = source;
	l.pos = 0;
	l.line = 0;
	return l;
}

bool lexer_has_next(const lexer* l) {
	return l->line < l->source->size - 1 || (l->line < l->source->size && l->pos < l->source->data[l->line].size);
}

static int next_char(lexer* l) {
	if (l->pos >= l->source->data[l->line].size) {
		l->pos = 0;
		l->line++;
		return '\n';
	}
	if (l->line >= l->source->size) {
		return -1;
	}
	return l->source->data[l->line].data[l->pos++];
}

static void skip_whitespace(lexer* l) {
	int c;
	do {
		c = next_char(l);
	} while ((c == ' ' || c == '\t' || c == '\n') && c != -1);
	l->pos--;
}

token lexer_next(lexer* l) {
	skip_whitespace(l);
	regmatch_t match;
	for (size_t i = 0; i < l->types->size; i++) {
		char* line = malloc(l->source->data[l->line].size - l->pos + 1);
		memcpy(line, l->source->data[l->line].data + l->pos, l->source->data[l->line].size - l->pos);
		line[l->source->data[l->line].size - l->pos] = '\0';
		if (regexec(&l->types->data[i].pattern, line, 1, &match, 0) == 0) {
			token t;
			t.type = i;
			const size_t length = match.rm_eo;
			t.value = malloc(length + 1);
			memcpy(t.value, line, length);
			t.value[length] = '\0';
			t.line = l->line;
			t.pos = l->pos;
			l->pos += length;
			return t;
		}
	}
	char* value = malloc(2);
	value[0] = l->source->data[l->line].data[l->pos];
	value[1] = '\0';
	l->pos++;
	return (token){.type = -1, .value = value, .line = l->line, .pos = l->pos};
}
