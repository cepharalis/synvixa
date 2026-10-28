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
				if (e->cursor.x == 0) {
					if (line_number != 0) {
						char_vector* previous_line = &e->file->lines.data[line_number - 1];
						e->cursor.x = previous_line->size;
						for (unsigned int i = 0; i < l->size; i++) {
							char_vector_push(previous_line, l->data[i]);
						}
						char_vector_vector_remove(&e->file->lines, line_number);
						e->cursor.y--;
					}
				}
				else {
					char_vector_remove(l, --e->cursor.x);
				}
			}
			else if (c == TERM_DELETE) {
				if (e->cursor.x < l->size) {
					char_vector_remove(l, e->cursor.x);
				}
				else if (line_number < e->file->lines.size - 1) {
					const char_vector* next_line = &e->file->lines.data[line_number + 1];
					for (unsigned int i = 0; i < next_line->size; i++) {
						char_vector_push(l, next_line->data[i]);
					}
					char_vector_vector_remove(&e->file->lines, line_number + 1);
				}
			}
			else if (c == TERM_LEFT_ARROW) {
				if (e->cursor.x > 0) {
					e->cursor.x--;
				}
				else if (e->cursor.y > 0) {
					e->cursor.y--;
					e->cursor.x = e->file->lines.data[line_number - 1].size;
				}
			}
			else if (c == TERM_RIGHT_ARROW) {
				if (e->cursor.x < l->size) {
					e->cursor.x++;
				}
				else if (e->cursor.y < e->file->lines.size - 1) {
					e->cursor.y++;
					e->cursor.x = 0;
				}
			}
			else if (c == TERM_UP_ARROW) {
				if (e->cursor.y > 0) {
					e->cursor.y--;
					const unsigned int line_size = e->file->lines.data[line_number - 1].size;
					if (e->cursor.x > line_size) {
						e->cursor.x = line_size;
					}
				}
			}
			else if (c == TERM_DOWN_ARROW) {
				if (e->cursor.y < e->file->lines.size - 1) {
					e->cursor.y++;
					const unsigned int line_size = e->file->lines.data[line_number + 1].size;
					if (e->cursor.x > line_size) {
						e->cursor.x = line_size;
					}
				}
			}
			else if (c == TERM_HOME) {
				e->cursor.x = 0;
			}
			else if (c == TERM_END) {
				e->cursor.x = l->size;
			}
			else if (c == '\n') {
				char_vector line;
				char_vector_init(&line, 0);
				char_vector_vector_insert(&e->file->lines, ++e->cursor.y, line);
				if (e->cursor.x != l->size) {
					char_vector* new_line = &e->file->lines.data[e->cursor.y];
					while (e->cursor.x < l->size) {
						char_vector_push(new_line, l->data[e->cursor.x]);
						char_vector_remove(l, e->cursor.x);
					}
				}
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

void editor_render(editor* e, const vec2 pos, const vec2 size) {
	if (e->cursor.x >= size.x + e->scroll.x) {
		e->scroll.x = e->cursor.x - size.x + 1;
	}
	else if (e->cursor.x < e->scroll.x) {
		e->scroll.x = e->cursor.x;
	}
	if (e->cursor.y >= size.y + e->scroll.y) {
		e->scroll.y = e->cursor.y - size.y + 1;
	}
	else if (e->cursor.y < e->scroll.y) {
		e->scroll.y = e->cursor.y;
	}
	for (unsigned int y = 0; y < size.y; y++) {
		const unsigned int line_number = y + e->scroll.y;
		term_set_cursor((vec2){.x = pos.x, .y = pos.y + y});
		if (line_number < e->file->lines.size) {
			const char_vector* l = &e->file->lines.data[line_number];
			const unsigned int line_pos = e->scroll.x;
			int length = (int) l->size - line_pos; // NOLINT(*-narrowing-conversions)
			if (length <= 0) {
				continue;
			}
			if (length > size.x) {
				length = (int) size.x;
			}
			term_write_l(l->data + line_pos, length);
			if (e->cursor.y == line_number) {
				unsigned int cursor_offset = 0;
				for (unsigned int i = 0; i < e->cursor.x; i++) {
					if (l->data[i] == '\t') {
						cursor_offset += 3;
					}
				}
				term_set_cursor((vec2){.x = pos.x + e->cursor.x - e->scroll.x + cursor_offset, .y = pos.y + y});
				if (e->cursor.x < l->size) {
					term_write_ls(l->data + e->cursor.x, 1, TERM_UNDERLINE);
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
