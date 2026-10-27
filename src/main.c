#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	bool render = true;
	vec2 termSize = term_get_size();
	unsigned int renderCount = 0;
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
			printf("RENDER %u\n", renderCount++);
			render = false;
		}
		usleep(50000);
	}
}