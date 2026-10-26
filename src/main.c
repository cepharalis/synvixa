#include "ui/editor.h"
#include "ui/term.h"

int main() {
	term_init();
	file f;
	const editor e = editor_new(&f);
	while (1) {
		term_clear(TERM_DEFAULT);
		editor_update(&e, (vec2){.x = 0, .y = 0}, term_get_size());
		term_render();
	}
}