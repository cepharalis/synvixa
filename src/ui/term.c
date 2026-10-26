#include "term.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
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
	write(STDOUT_FILENO, ANSI_ENABLE_BUFFER "\033[?1007l\033[?1000l\033[?1002l\033[?1003l\033[?1006l", sizeof(ANSI_ENABLE_BUFFER) + sizeof("\033[?1007l\033[?1000l\033[?1002l\033[?1003l\033[?1006l") - 2);
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
		const unsigned int index = cursor.x + cursor.y * size.x;
		if (index >= size.x * size.y) {
			return;
		}
		buffer[index].c = c;
		buffer[index].colour = colour;
		buffer[index].background = background;
		buffer[index].style = style;
		cursor.x++;
		s++;
	}
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
