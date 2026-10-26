#pragma once

#include "term.h"
#include "file/file.h"

typedef struct editor {
	file* file;
	vec2 cursor;
	vec2 scroll;
} editor;

editor editor_new(file* f);

void editor_update(const editor* e, vec2 pos, vec2 size);