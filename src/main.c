#include <stdlib.h>

#include "ui/editor.h"
#include "ui/term.h"

int main() {
	term_init();
	file f;
	f.line_count = 5;
	f.line_capacity = 10;
	f.lines = malloc(sizeof(line) * f.line_capacity);
	for (unsigned int i = 0; i < f.line_count; i++) {
		f.lines[i].text_capacity = 16;
		f.lines[i].text = malloc(f.lines[i].text_capacity);
		f.lines[i].text_length = 0;
	}
	editor e = editor_new(&f);
	while (1) {
		term_clear(TERM_DEFAULT);
		editor_update(&e, (vec2){.x = 0, .y = 0}, term_get_size());
		term_render();
	}
}