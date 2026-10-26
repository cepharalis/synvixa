#include "term.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
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
		buffer[i].colour = TERM_WHITE;
		buffer[i].background = colour;
		buffer[i].style = TERM_NONE;
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
	term_write_cbs(s, TERM_WHITE, TERM_BLACK, TERM_NONE);
}

void term_write_c(const char* s, const unsigned int colour) {
	term_write_cbs(s, colour, TERM_BLACK, TERM_NONE);
}

void term_write_cb(const char* s, const unsigned int colour, const unsigned int background) {
	term_write_cbs(s, colour, background, TERM_NONE);
}

void term_write_cbs(const char* s, const unsigned int colour, const unsigned int background, const unsigned int style) {
	const unsigned int startX = cursor.x;
	while (*s) {
		const char c = *s;
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
		s++;
	}
}

int term_read() {
	return getchar();
}

void term_render() {
	write(STDOUT_FILENO, ANSI_CURSOR_POS, sizeof(ANSI_CURSOR_POS) - 1);
	char b[7];
	unsigned int index = 0;
	for (unsigned int i = 0; i < size.x * size.y; i++) {
		unsigned char colour = buffer[i].colour;
		if (colour & TERM_BRIGHT) {
			colour &= ~TERM_BRIGHT;
			colour += 60;
		}
		b[index++] = '\x1b';
		b[index++] = '[';
		index += term_char_to_str(b + index, colour);
		b[index++] = 'm';
		b[index++] = buffer[i].c;
		write(STDOUT_FILENO, b, index);
		index = 0;
	}
	struct winsize termSize;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize) == 0) {
		size.x = termSize.ws_col;
		size.y = termSize.ws_row;
		free(buffer);
		buffer = malloc(size.x * size.y * sizeof(Char));
	}
}

void term_destroy() {
	if (!buffer) {
		return;
	}
	free(buffer);
	write(STDOUT_FILENO, ANSI_DISABLE_BUFFER ANSI_ENABLE_LINE_WRAP, sizeof(ANSI_DISABLE_BUFFER) + sizeof(ANSI_ENABLE_LINE_WRAP) - 2);
}
