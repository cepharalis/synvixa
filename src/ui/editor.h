#pragma once

#include "term.h"
#include "file/file.h"

#define EDITOR_NORMAL TERM_WHITE
#define EDITOR_KEYWORD TERM_BLUE
#define EDITOR_STRING TERM_GREEN
#define EDITOR_COMMENT (TERM_BLACK | TERM_BRIGHT)
#define EDITOR_CONSTANT (TERM_BLUE | TERM_BRIGHT)
#define EDITOR_PREPROCESSOR TERM_YELLOW
#define EDITOR_ERROR TERM_RED

#define EDITOR_PACKET_BASE 1
#define EDITOR_PACKET_ADD_CHAR (EDITOR_PACKET_BASE)
#define EDITOR_PACKET_ADD_LINE (EDITOR_PACKET_BASE + 1)
#define EDITOR_PACKET_REMOVE_CHAR (EDITOR_PACKET_BASE + 2)
#define EDITOR_PACKET_BACKSPACE (EDITOR_PACKET_BASE + 3)
#define EDITOR_PACKET_END (EDITOR_PACKET_BASE + 3)

typedef struct editor {
	file* file;
	token_vector tokens;
	vec2 cursor;
	vec2 scroll;
	bool dirty;
	int movePages;
} editor;

editor editor_new(file* f);

bool editor_update(editor* e, const net_socket* sockets, int socket_count);

void editor_process_packet(editor* e, char id, const char* packet);

void editor_render(editor* e, vec2 pos, vec2 size);