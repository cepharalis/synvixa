#include "editor.h"

editor editor_new(file* f) {
	editor e;
	e.file = f;
	e.cursor = (vec2){.x = 0, .y = 0};
	e.scroll = (vec2){.x = 0, .y = 0};
	return e;
}

void editor_update(const editor* e, const vec2 pos, const vec2 size) {
	for (unsigned int y = 0; y < size.y; y++) {
		const unsigned int lineNumber = y + e->scroll.y;
		term_set_cursor((vec2){.x = pos.x, .y = pos.y + y});
		if (lineNumber < e->file->line_count) {
			const line* l = &e->file->lines[lineNumber];
			const unsigned int linePos = e->scroll.x;
			if (linePos < l->text_length) {
				term_write(l->text + linePos);
			}
		}
		else {
			term_write_c("~", TERM_BLUE | TERM_BRIGHT);
		}
	}
}
