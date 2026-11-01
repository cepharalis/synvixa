#include "lexer.h"

token_type token_type_new(char* name, const char* pattern, const token_group group) {
	char* p = malloc(strlen(pattern) + 2);
	p[0] = '^';
	strcpy(p + 1, pattern);
	const regex r = regex_compile(p);
	return (token_type){.name = name, .pattern = r, .group = group};
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

static void skip_whitespace(lexer* l) {
	while (l->line < l->source->size) {
		const char_vector* line = &l->source->data[l->line];
		if (l->pos >= line->size) {
			l->pos = 0;
			l->line++;
			continue;
		}
		const char c = line->data[l->pos];
		if (c == ' ' || c == '\t' || c == '\n') {
			l->pos++;
		}
		else {
			break;
		}
	}
}

token lexer_next(lexer* l) {
	skip_whitespace(l);
	if (l->line >= l->source->size) {
		return (token){.type = -1, .value = nullptr, .line = l->line, .pos = l->pos, .length = 0};
	}
	for (size_t i = 0; i < l->types->size; i++) {
		char* line = malloc(l->source->data[l->line].size - l->pos + 1);
		memcpy(line, l->source->data[l->line].data + l->pos, l->source->data[l->line].size - l->pos);
		line[l->source->data[l->line].size - l->pos] = '\0';
		const regex_match match = regex_find(&l->types->data[i].pattern, line);
		if (match.match) {
			token t;
			t.type = i;
			const size_t length = match.end;
			t.value = malloc(length + 1);
			memcpy(t.value, line, length);
			free(line);
			t.value[length] = '\0';
			t.line = l->line;
			t.pos = l->pos;
			l->pos += length;
			t.length = length;
			return t;
		}
		free(line);
	}
	char* value = malloc(2);
	value[0] = l->source->data[l->line].data[l->pos];
	value[1] = '\0';
	l->pos++;
	return (token){.type = -1, .value = value, .line = l->line, .pos = l->pos, .length = 1};
}
