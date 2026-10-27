#include "editor.h"

editor editor_new(file* f) {
	editor e;
	e.file = f;
	e.cursor = (vec2){.x = 0, .y = 0};
	e.scroll = (vec2){.x = 0, .y = 0};
	return e;
}

bool editor_update(editor* e) {
	int c;
	bool dirty = false;
	while ((c = term_read()) != TERM_EOF) {
		const unsigned int line_number = e->cursor.y;
		if (line_number < e->file->lines.size) {
			char_vector* l = &e->file->lines.data[line_number];
			if (c == TERM_BACKSPACE) {
				if (e->cursor.x == l->size && l->size > 0) {
					l->data[--l->size] = '\0';
					e->cursor.x--;
				}
				else if (e->cursor.x > 0) {
					char_vector_remove(l, e->cursor.x - 1);
					e->cursor.x--;
				}
			}
			else if (c == TERM_DELETE) {
				if (e->cursor.x == l->size - 1) {
					l->data[--l->size] = '\0';
				}
				else if (e->cursor.x != l->size) {
					char_vector_remove(l, e->cursor.x);
				}
			}
			else if (c == TERM_LEFT_ARROW) {
				if (e->cursor.x > 0) {
					e->cursor.x--;
				}
			}
			else if (c == TERM_RIGHT_ARROW) {
				if (e->cursor.x < l->size) {
					e->cursor.x++;
				}
			}
			else if (c == TERM_HOME) {
				e->cursor.x = 0;
			}
			else if (c == TERM_END) {
				e->cursor.x = l->size;
			}
			else if (c == '\n') {
				e->cursor.y++;
				e->cursor.x = 0;
			}
			else {
				if (e->cursor.x == l->size) {
					char_vector_push(l, (char) c);
				}
				else {
					char_vector_insert(l, e->cursor.x, (char) c);
				}
				e->cursor.x++;
			}
		}
		dirty = true;
	}
	return dirty;
}

void editor_render(const editor* e, const vec2 pos, const vec2 size) {
	for (unsigned int y = 0; y < size.y; y++) {
		const unsigned int line_number = y + e->scroll.y;
		term_set_cursor((vec2){.x = pos.x, .y = pos.y + y});
		if (line_number < e->file->lines.size) {
			const char_vector* l = &e->file->lines.data[line_number];
			const unsigned int line_pos = e->scroll.x;
			if (line_pos < l->size) {
				term_write(l->data + line_pos);
			}
			if (e->cursor.y == line_number) {
				const unsigned int cursor_x = e->cursor.x;
				term_set_cursor((vec2){.x = pos.x + cursor_x, .y = pos.y + y});
				if (cursor_x < l->size) {
					char cursor_chars[2];
					cursor_chars[0] = *(l->data + cursor_x);
					cursor_chars[1] = '\0';
					term_write_s(cursor_chars, TERM_UNDERLINE);
				}
				else {
					term_write_s(" ", TERM_UNDERLINE);
				}
			}
		}
		else {
			term_write_c("~", TERM_BLUE | TERM_BRIGHT);
		}
	}
}
