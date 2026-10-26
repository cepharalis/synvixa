#pragma once

#include <stdint.h>

#define ANSI_ENABLE_BUFFER "\x1b[?1049h"
#define ANSI_DISABLE_BUFFER "\x1b[?1049l"
#define ANSI_ENABLE_LINE_WRAP "\x1b[7h"
#define ANSI_DISABLE_LINE_WRAP "\x1b[7l"
#define ANSI_RESET "\x1b[0m"
#define ANSI_CURSOR_POS "\x1b[H"

#define TERM_DEFAULT 0

#define TERM_BLACK 30
#define TERM_RED 31
#define TERM_GREEN 32
#define TERM_YELLOW 33
#define TERM_BLUE 34
#define TERM_MAGENTA 35
#define TERM_CYAN 36
#define TERM_WHITE 37
#define TERM_BRIGHT 128

#define TERM_BOLD 1
#define TERM_ITALIC 3
#define TERM_UNDERLINE 4

typedef struct vec2 {
	uint32_t x;
	uint32_t y;
} vec2;

int term_init();

vec2 term_get_size();

void term_set_cursor(vec2 pos);

vec2 term_get_cursor();

void term_clear(unsigned int colour);

unsigned int term_char_to_str(char* b, unsigned char c);

void term_write(const char* s);

void term_write_c(const char* s, unsigned int colour);

void term_write_cb(const char* s, unsigned int colour, unsigned int background);

void term_write_cbs(const char* s, unsigned int colour, unsigned int background, unsigned int style);

int term_read();

void term_render();

void term_destroy();