#include "ui/term.h"

int main() {
	term_init();
	term_clear(TERM_BLACK);
	term_write_c("Hello World!", TERM_RED);
	term_render();
	while (1) {}
}