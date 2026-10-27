#include <unistd.h>

#include "ui/editor.h"
#include "ui/term.h"

int main() {
	term_init();
	file f;
	char_vector_vector_init(&f.lines, 8);
	char_vector line;
	char_vector_init(&line, 0);
	char_vector_vector_push(&f.lines, line);
	editor e = editor_new(&f);
	bool render = true;
	vec2 termSize = term_get_size();
	while (1) {
		render |= editor_update(&e);
		const vec2 newSize = term_get_size();
		if (termSize.x != newSize.x || termSize.y != newSize.y) {
			termSize = newSize;
			render = true;
		}
		if (render) {
			term_clear(TERM_DEFAULT);
			editor_render(&e, (vec2){.x = 0, .y = 0}, term_get_size());
			term_render();
			render = false;
		}
		usleep(50000);
	}
}