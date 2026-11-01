#include "term.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static term_char* buffer;
static vec2 size;
static vec2 cursor;

#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>

static vec2 get_term_size() {
	struct winsize term_size;
	auto s = (vec2){.x = 0, .y = 0};
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size) == 0) {
		s.x = term_size.ws_col;
		s.y = term_size.ws_row;
	}
	return s;
}

static void resize_handler(int) {
	size = get_term_size();
	free(buffer);
	buffer = malloc(size.x * size.y * sizeof(term_char));
}
#elifdef _WIN32
#include <conio.h>
#include <windows.h>

static vec2 get_term_size() {
	CONSOLE_SCREEN_BUFFER_INFO buffer_info;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info) == 0) {
		write(STDOUT_FILENO, "\x1b[18t", sizeof("\x1b[18t") - 1);
		char b[32];
		for (int i = 0; i < 32; i++) {
			int c;
			while ((c = getchar()) == EOF) {}
			b[i] = (char) c;
			if (b[i] == 't') {
				b[i + 1] = '\0';
				vec2 s;
				sscanf(b, "\x1b[8;%d;%dt", &s.x, &s.y); // NOLINT(*-err34-c)
				return s;
			}
		}
		return (vec2){.x = 0, .y = 0};
	}
	return (vec2){.x = buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1, .y = buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1};
}
#else
#error Unsupported platform
#endif

int term_init() {
#ifdef _WIN32
	// ReSharper disable once CppLocalVariableMayBeConst
	HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD outputMode;
	GetConsoleMode(outputHandle, &outputMode);
	SetConsoleMode(outputHandle, outputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
	size = get_term_size();
	buffer = malloc(size.x * size.y * sizeof(term_char));
	write(STDOUT_FILENO, ANSI_ENABLE_BUFFER "\x1b[?1007l\x1b[?1000l\x1b[?1002l\x1b[?1003l\x1b[?1006l\x1b[?25l", sizeof(ANSI_ENABLE_BUFFER) + sizeof("\x1b[?1007l\x1b[?1000l\x1b[?1002l\x1b[?1003l\x1b[?1006l\x1b[?25l") - 2);
#if defined(__linux__) || defined(__APPLE__)
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
	signal(SIGWINCH, resize_handler);
#elifdef _WIN32
	// ReSharper disable once CppLocalVariableMayBeConst
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(h, &mode);
	SetConsoleMode(h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
#else
#error Unsupported platform
#endif
	term_clear(TERM_DEFAULT);
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

void term_clear_line(const uint32_t line) {
	for (unsigned int x = 0; x < size.x; x++) {
		buffer[line * size.x + x].c = ' ';
		buffer[line * size.x + x].colour = TERM_DEFAULT;
		buffer[line * size.x + x].background = TERM_DEFAULT;
		buffer[line * size.x + x].style = TERM_DEFAULT;
	}
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

void term_write_cs(const char* s, const unsigned int colour, const unsigned int style) {
	term_write_lcbs(s, strlen(s), colour, TERM_DEFAULT, style);
}

void term_write_cbs(const char* s, const unsigned int colour, const unsigned int background, const unsigned int style) {
	term_write_lcbs(s, strlen(s), colour, background, style);
}

void term_write_l(const char* s, const unsigned int length) {
	term_write_lcbs(s, length, TERM_DEFAULT, TERM_DEFAULT, TERM_DEFAULT);
}

void term_write_lc(const char* s, const unsigned int length, const unsigned int colour) {
	term_write_lcbs(s, length, colour, TERM_DEFAULT, TERM_DEFAULT);
}

void term_write_ls(const char* s, const unsigned int length, const unsigned int style) {
	term_write_lcbs(s, length, TERM_DEFAULT, TERM_DEFAULT, style);
}

void term_write_lcs(const char* s, const unsigned int length, const unsigned int colour, const unsigned int style) {
	term_write_lcbs(s, length, colour, TERM_DEFAULT, style);
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
			const unsigned int index = cursor.x + cursor.y * size.x;
			for (int j = 0; j < 4; j++) {
				buffer[index + j].c = ' ';
				buffer[index + j].colour = colour;
				buffer[index + j].background = background;
				buffer[index + j].style = style;
			}
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

void term_write_char(const term_char c) {
	buffer[cursor.x + cursor.y * size.x] = c;
	cursor.x++;
}

term_char term_read_buffer() {
	return buffer[cursor.x + cursor.y * size.x];
}

int term_read() {
#if defined(__linux__) || defined(__APPLE__)
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
		case '5':
			c = getchar();
			if (c == '~') {
				return TERM_PAGE_UP;
			}
			return TERM_EOF;
		case '6':
			c = getchar();
			if (c == '~') {
				return TERM_PAGE_DOWN;
			}
			return TERM_EOF;
		default:
			return TERM_EOF;
	}
#elifdef _WIN32
	if (!_kbhit()) {
		return TERM_EOF;
	}
	int c = _getch();
	if (c == 0 || c == 224) {
		c = _getch();
		switch (c) {
			case 72:
				return TERM_UP_ARROW;
			case 80:
				return TERM_DOWN_ARROW;
			case 77:
				return TERM_RIGHT_ARROW;
			case 75:
				return TERM_LEFT_ARROW;
			case 71:
				return TERM_HOME;
			case 79:
				return TERM_END;
			case 83:
				return TERM_DELETE;
			case 73:
				return TERM_PAGE_UP;
			case 81:
				return TERM_PAGE_DOWN;
			default:
				return TERM_EOF;
		}
	}
	if (c == 8) {
		return TERM_BACKSPACE;
	}
	if (c == 13) {
		return '\n';
	}
	return c;
#else
#error Unsupported platform
#endif
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
	ssize_t written;
	size_t offset = 0;
	while ((written = write(STDOUT_FILENO, b + offset, index - offset)) != index - offset) {
		if (written > 0) {
			offset += written;
		}
	}
#ifdef _WIN32
	const vec2 new_size = get_term_size();
	if (new_size.x != size.x || new_size.y != size.y) {
		size = new_size;
		free(buffer);
		buffer = malloc(size.x * size.y * sizeof(term_char));
	}
#endif
}

void term_destroy() {
	if (!buffer) {
		return;
	}
	free(buffer);
	write(STDOUT_FILENO, ANSI_DISABLE_BUFFER ANSI_ENABLE_LINE_WRAP, sizeof(ANSI_DISABLE_BUFFER) + sizeof(ANSI_ENABLE_LINE_WRAP) - 2);
}
