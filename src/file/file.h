#pragma once

typedef struct line {
	char* text;
	int text_length;
	int text_capacity;
} line;

typedef struct file {
	char* path;
	line* lines;
	int line_count;
	int line_capacity;
} file;