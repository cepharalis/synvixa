#include <stdio.h>

#include "ui/term.h"

int main() {
	term_init();
	char buffer[1024];
	int length = 0;
	while (1) {
		term_clear(TERM_BLACK);
		term_write_c(buffer, TERM_BLUE | TERM_BRIGHT);
		term_render();
		int input;
		while ((input = term_read()) != EOF) {
			if (input == 127) {
				if (length > 0) {
					buffer[--length] = '\0';
				}
			}
			else {
				buffer[length++] = (char) input;
				buffer[length] = '\0';
			}
		}
	}
}