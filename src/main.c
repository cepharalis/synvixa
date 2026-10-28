#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "ui/editor.h"
#include "ui/term.h"

static volatile sig_atomic_t running = 1;

static void handle_sigint(int) {
	running = 0;
}

int main(const int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	struct sigaction action;
	action.sa_handler = handle_sigint;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	if (sigaction(SIGINT, &action, nullptr) < 0) {
		printf("Error setting up signal handler\n");
		return 1;
	}
	term_init();
	file f = file_open(argv[1]);
	if (f.lines.size == 0) {
		char_vector line;
		char_vector_init(&line, 0);
		char_vector_vector_push(&f.lines, line);
	}
	editor e = editor_new(&f);
	bool render = true;
	vec2 termSize = term_get_size();
	while (running) {
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
	term_destroy();
	return 0;
}