// This comment was written with synvixa

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "net/file_transfer/fileSR.h"
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
		printf("Usage: %s <ip address> [filename]\n", argv[0]);
		return 1;
	}
	signal(SIGINT, handle_sigint);
	plugins_load();
	term_init();
	char* ip = argv[1];
	file f;
	if (argc > 2) {
		f = file_read(argv[2]);
	}
	else {
		f = file_read("temp.txt");
		f.lines.size = 0;
	}
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
	editor e = editor_new(&f);
	bool render = true;
	vec2 termSize = term_get_size();
	while (running) {
		if (fileSR_receive(f.path, ip, 8888) != -1) {
			f = file_read(f.path);
			render = true;
		}
		if (editor_update(&e)) {
			render = true;
			file_write(&f);
			fileSR_send(f.path, ip, 8888);
		}
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