#include "lexer.h"

lexer lexer_new(token_type_vector* types, char_vector_vector* source) {
	lexer l;
	l.types = types;
	l.source = source;
	l.pos = 0;
	l.line = 0;
	return l;
}

static char next_char(lexer* l) {
	if (l->pos == l->source->data[l->line].size) {
		l->pos = 0;
		l->line++;
		return '\n';
	}
	return l->source->data[l->line].data[l->pos++];
}

static void skip_whitespace(lexer* l) {
	char c;
	do {
		c = next_char(l);
	} while (c == ' ' || c == '\t' || c == '\n');
	l->pos--;
}

token lexer_next(lexer* l) {
	skip_whitespace(l);
	regmatch_t match;
	for (size_t i = 0; i < l->types->size; i++) {
		if (regexec(&l->types->data[i].pattern, l->source->data[l->line].data + l->pos, 1, &match, 0) == 0) {
			token t;
			t.type = i;
			const size_t length = match.rm_eo - match.rm_so;
			t.value = malloc(length + 1);
			strncpy(t.value, l->source->data[l->line].data + l->pos + match.rm_so, length);
			t.value[length] = '\0';
			t.line = l->line;
			t.pos = l->pos + match.rm_so;
			return t;
		}
	}
	char* value = malloc(2);
	value[0] = l->source->data[l->line].data[l->pos];
	value[1] = '\0';
	return (token){.type = -1, .value = value, .line = l->line, .pos = l->pos};
}
