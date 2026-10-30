// This comment was written with synvixa

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "plugin/plugins.h"
#include "ui/editor.h"
#include "ui/term.h"
#include "util/string_util.h"

static volatile sig_atomic_t running = 1;

static void handle_sigint(int) {
	running = 0;
}

int main(const int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	signal(SIGINT, handle_sigint);
	plugins_load();
	//term_init();
	file f = file_read(argv[1]);
	if (f.lines.size == 0) {
		char_vector line;
		char_vector_init(&line, 0);
		char_vector_vector_push(&f.lines, line);
	}
	const plugin_vector* plugins = plugins_get();
	f.language = nullptr;
	for (int i = 0; i < plugins->size; i++) {
		const language_vector* languages = &plugins->data[i].languages;
		for (int j = 0; j < languages->size; j++) {
			language* language = &languages->data[j];
			char** extensions = language->extensions;
			for (int k = 0; k < language->extension_count; k++) {
				if (string_ends_with(f.path, extensions[k])) {
					f.language = language;
					break;
				}
			}
		}
	}
	lexer l = lexer_new(&f.language->token_types, &f.lines);
	while (lexer_has_next(&l)) {
		const token t = lexer_next(&l);
		if (t.type != -1) {
			printf("%s, %s, length=%lu\n", f.language->token_types.data[t.type].name, t.value, strlen(t.value));
		}
		else {
			printf("Unknown token, %s\n", t.value);
		}
	}
	return 0;
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
	file_write(&f);
	term_destroy();
	plugins_unload();
	return 0;
}