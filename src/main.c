#include "ui/term.h"

int main() {
	term_init();
	vec2 pos = {.x = 3, .y = 20};
	vec2 velocity = {.x = 1, .y = 1};
	while (1) {
		term_clear(TERM_BLACK);
		term_set_cursor(pos);
		term_write_c("Hello World!", TERM_BLUE | TERM_BRIGHT);
		term_render();
		pos.x += velocity.x;
		pos.y += velocity.y;
		const vec2 term_size = term_get_size();
		if (pos.y <= 0 || pos.y >= term_size.y - 1) {
			velocity.y *= -1;
		}
		if (pos.x <= 0 || pos.x >= term_size.x - 12) {
			velocity.x *= -1;
		}
	}
}