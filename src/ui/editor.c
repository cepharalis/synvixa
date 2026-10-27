#include "editor.h"

#include <stdlib.h>

editor editor_new(file* f) {
	editor e;
	e.file = f;
	e.cursor = (vec2){.x = 0, .y = 0};
	e.scroll = (vec2){.x = 0, .y = 0};
	return e;
}

void editor_update(editor* e, const vec2 pos, const vec2 size) {
	int c;
	while ((c = term_read()) != TERM_EOF) {
		const unsigned int line_number = e->cursor.y;
		if (line_number < e->file->line_count) {
			line* l = &e->file->lines[line_number];
			if (c == TERM_BACKSPACE) {
				if (e->cursor.x == l->text_length && l->text_length > 0) {
					l->text[--l->text_length] = '\0';
					e->cursor.x--;
				}
				else if (e->cursor.x > 0) {
					for (unsigned int i = e->cursor.x; i <= l->text_length; i++) {
						l->text[i - 1] = l->text[i];
					}
					l->text_length--;
					e->cursor.x--;
				}
			}
			else if (c == TERM_DELETE) {
				if (e->cursor.x == l->text_length - 1) {
					l->text[--l->text_length] = '\0';
				}
				else if (e->cursor.x != l->text_length) {
					for (unsigned int i = e->cursor.x + 1; i <= l->text_length; i++) {
						l->text[i - 1] = l->text[i];
					}
					l->text_length--;
				}
			}
			else if (c == TERM_LEFT_ARROW) {
				if (e->cursor.x > 0) {
					e->cursor.x--;
				}
			}
			else if (c == TERM_RIGHT_ARROW) {
				if (e->cursor.x < l->text_length) {
					e->cursor.x++;
				}
			}
			else if (c == TERM_HOME) {
				e->cursor.x = 0;
			}
			else if (c == TERM_END) {
				e->cursor.x = l->text_length;
			}
			else {
				if (l->text_length + 1 == l->text_capacity) {
					l->text_capacity *= 2;
					l->text = realloc(l->text, l->text_capacity);
				}
				if (e->cursor.x == l->text_length) {
					l->text[l->text_length++] = (char) c;
					l->text[l->text_length] = '\0';
				}
				else {
					for (unsigned int i = l->text_length; i > e->cursor.x; i--) {
						l->text[i] = l->text[i - 1];
					}
					l->text[e->cursor.x] = (char) c;
					l->text_length++;
				}
				e->cursor.x++;
			}
		}
	}
	for (unsigned int y = 0; y < size.y; y++) {
		const unsigned int line_number = y + e->scroll.y;
		term_set_cursor((vec2){.x = pos.x, .y = pos.y + y});
		if (line_number < e->file->line_count) {
			const line* l = &e->file->lines[line_number];
			const unsigned int line_pos = e->scroll.x;
			if (line_pos < l->text_length) {
				term_write(l->text + line_pos);
			}
			if (e->cursor.y == line_number) {
				const unsigned int cursor_x = e->cursor.x;
				term_set_cursor((vec2){.x = pos.x + cursor_x, .y = pos.y + y});
				if (cursor_x < l->text_length) {
					char cursor_chars[2];
					cursor_chars[0] = *(l->text + cursor_x);
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
