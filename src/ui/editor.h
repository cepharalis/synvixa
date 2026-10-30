#pragma once

#include "term.h"
#include "file/file.h"

#define EDITOR_NORMAL TERM_WHITE
#define EDITOR_KEYWORD TERM_BLUE
#define EDITOR_STRING TERM_GREEN
#define EDITOR_COMMENT (TERM_BLACK | TERM_BRIGHT)
#define EDITOR_CONSTANT (TERM_BLUE | TERM_BRIGHT)
#define EDITOR_ERROR TERM_RED

typedef struct editor {
	file* file;
	token_vector tokens;
	vec2 cursor;
	vec2 scroll;
	bool dirty;
} editor;

editor editor_new(file* f);

bool editor_update(editor* e);

void editor_render(editor* e, vec2 pos, vec2 size);