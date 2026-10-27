#pragma once

#include "term.h"
#include "file/file.h"

typedef struct editor {
	file* file;
	vec2 cursor;
	vec2 scroll;
} editor;

editor editor_new(file* f);

bool editor_update(editor* e);

void editor_render(const editor* e, vec2 pos, vec2 size);