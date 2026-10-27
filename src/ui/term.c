#include "term.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef struct Char {
	char c;
	unsigned char colour;
	unsigned char background;
	unsigned char style;
} Char;

static Char* buffer;
static vec2 size;
static vec2 cursor;

static void resize_handler(int) {
	struct winsize termSize;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize) == 0) {
		size.x = termSize.ws_col;
		size.y = termSize.ws_row;
		free(buffer);
		buffer = malloc(size.x * size.y * sizeof(Char));
	}
}

int term_init() {
	struct winsize termSize;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize) != 0) {
		return -1;
	}
	size.x = termSize.ws_col;
	size.y = termSize.ws_row;
	buffer = malloc(size.x * size.y * sizeof(Char));
	write(STDOUT_FILENO, ANSI_ENABLE_BUFFER "\x1b[?1007l\x1b[?1000l\x1b[?1002l\x1b[?1003l\x1b[?1006l\x1b[?25l", sizeof(ANSI_ENABLE_BUFFER) + sizeof("\x1b[?1007l\x1b[?1000l\x1b[?1002l\x1b[?1003l\x1b[?1006l\x1b[?25l") - 2);
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
	term_clear(TERM_DEFAULT);
	signal(SIGWINCH, resize_handler);
	return 0;
}

vec2 term_get_size() {
	return size;
}

void term_set_cursor(const vec2 pos) {
	cursor = pos;
}

vec2 term_get_cursor() {
	return cursor;
}

void term_clear(const unsigned int colour) {
	for (unsigned int i = 0; i < size.x * size.y; i++) {
		buffer[i].c = ' ';
		buffer[i].colour = TERM_DEFAULT;
		buffer[i].background = colour;
		buffer[i].style = TERM_DEFAULT;
	}
	cursor = (vec2){.x = 0, .y = 0};
}

unsigned int term_char_to_str(char* b, const unsigned char c) {
	if (c >= 100) {
		b[0] = (char) (c / 100 + '0');
		b[1] = (char) (c / 10 % 10 + '0');
		b[2] = (char) (c % 10 + '0');
		return 3;
	}
	if (c >= 10) {
		b[0] = (char) (c / 10 + '0');
		b[1] = (char) (c % 10 + '0');
		return 2;
	}
	b[0] = (char) (c + '0');
	return 1;
}

void term_write(const char* s) {
	term_write_lcbs(s, strlen(s), TERM_DEFAULT, TERM_DEFAULT, TERM_DEFAULT);
}

void term_write_c(const char* s, const unsigned int colour) {
	term_write_lcbs(s, strlen(s), colour, TERM_DEFAULT, TERM_DEFAULT);
}

void term_write_s(const char* s, const unsigned int style) {
	term_write_lcbs(s, strlen(s), TERM_DEFAULT, TERM_DEFAULT, style);
}

void term_write_cb(const char* s, const unsigned int colour, const unsigned int background) {
	term_write_lcbs(s, strlen(s), colour, background, TERM_DEFAULT);
}

void term_write_cbs(const char* s, const unsigned int colour, const unsigned int background, const unsigned int style) {
	term_write_lcbs(s, strlen(s), colour, background, style);
}

void term_write_l(const char* s, const unsigned int length) {
	term_write_lcbs(s, length, TERM_DEFAULT, TERM_DEFAULT, TERM_DEFAULT);
}

void term_write_ls(const char* s, const unsigned int length, const unsigned int style) {
	term_write_lcbs(s, length, TERM_DEFAULT, TERM_DEFAULT, style);
}

void term_write_lcbs(const char* s, const unsigned int length, const unsigned int colour, const unsigned int background, const unsigned int style) {
	const unsigned int startX = cursor.x;
	for (unsigned int i = 0; i < length; i++) {
		const char c = s[i];
		if (c == '\n') {
			cursor.y++;
			cursor.x = startX;
		}
		else if (c == '\t') {
			cursor.x += 4;
		}
		else {
			const unsigned int index = cursor.x + cursor.y * size.x;
			if (index >= size.x * size.y) {
				return;
			}
			buffer[index].c = c;
			buffer[index].colour = colour;
			buffer[index].background = background;
			buffer[index].style = style;
			cursor.x++;
		}
	}
}

int term_read() {
	int c = getchar();
	if (c == EOF) {
		return TERM_EOF;
	}
	if (c == 127 || c == 8) {
		return TERM_BACKSPACE;
	}
	if (c != '\033') {
		return c;
	}
	c = getchar();
	if (c != '[') {
		return TERM_EOF;
	}
	c = getchar();
	switch (c) {
		case 'A':
			return TERM_UP_ARROW;
		case 'B':
			return TERM_DOWN_ARROW;
		case 'C':
			return TERM_RIGHT_ARROW;
		case 'D':
			return TERM_LEFT_ARROW;
		case 'H':
			return TERM_HOME;
		case 'F':
			return TERM_END;
		case 'P':
			return TERM_DELETE;
		case '3':
			c = getchar();
			if (c == '~') {
				return TERM_DELETE;
			}
			return TERM_EOF;
		default:
			return TERM_EOF;
	}
}

void term_render() {
	char b[size.x * size.y * 19];
	unsigned int index = 0;
	unsigned char previousStyle = TERM_DEFAULT;
	unsigned char previousColour = TERM_DEFAULT;
	unsigned char previousBackground = TERM_DEFAULT;
	for (unsigned int y = 0; y < size.y; y++) {
		b[index++] = '\x1b';
		b[index++] = '[';
		index += term_char_to_str(b + index, y + 1);
		b[index++] = ';';
		b[index++] = '1';
		b[index++] = 'H';

		for (unsigned int x = 0; x < size.x; x++) {
			const unsigned int i = x + y * size.x;

			const unsigned char style = buffer[i].style;
			unsigned char colour = buffer[i].colour;
			if (colour & TERM_BRIGHT) {
				colour &= ~TERM_BRIGHT;
				colour += 60;
			}
			unsigned char background = buffer[i].background;
			if (background & TERM_BRIGHT) {
				background &= ~TERM_BRIGHT;
				background += 60;
			}

			if (i == 0 || previousStyle != style || previousColour != colour || previousBackground != background) {
				b[index++] = '\x1b';
				b[index++] = '[';
				b[index++] = '0';
				b[index++] = 'm';
				b[index++] = '\x1b';
				b[index++] = '[';
				if (style != TERM_DEFAULT) {
					index += term_char_to_str(b + index, style);
					b[index++] = ';';
				}
				if (colour != TERM_DEFAULT) {
					index += term_char_to_str(b + index, colour);
					b[index++] = ';';
				}
				if (background != TERM_DEFAULT) {
					index += term_char_to_str(b + index, background + 10);
					b[index++] = ';';
				}
				b[index - 1] = 'm';

				previousStyle = style;
				previousColour = colour;
				previousBackground = background;
			}

			b[index++] = buffer[i].c;
		}
	}
	write(STDOUT_FILENO, b, index);
}

void term_destroy() {
	if (!buffer) {
		return;
	}
	free(buffer);
	write(STDOUT_FILENO, ANSI_DISABLE_BUFFER ANSI_ENABLE_LINE_WRAP, sizeof(ANSI_DISABLE_BUFFER) + sizeof(ANSI_ENABLE_LINE_WRAP) - 2);
}
