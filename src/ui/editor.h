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
#define EDITOR_PACKET_SET_CURSOR (EDITOR_PACKET_BASE + 4)
#define EDITOR_PACKET_REMOVE_CURSOR (EDITOR_PACKET_BASE + 5)
#define EDITOR_PACKET_END (EDITOR_PACKET_BASE + 5)

constexpr unsigned int CURSOR_COLOURS[17] = {
	TERM_RED, TERM_YELLOW, TERM_GREEN, TERM_CYAN,
	TERM_BLUE, TERM_MAGENTA, TERM_BLACK, TERM_RED | TERM_BRIGHT,
	TERM_YELLOW | TERM_BRIGHT, TERM_GREEN | TERM_BRIGHT, TERM_CYAN | TERM_BRIGHT, TERM_BLUE | TERM_BRIGHT,
	TERM_MAGENTA | TERM_BRIGHT, TERM_BLACK | TERM_BRIGHT, TERM_RED, TERM_YELLOW,
	TERM_WHITE
};

typedef struct editor {
	file* file;
	token_vector tokens;
	vec2 cursor;
	vec2 scroll;
	bool dirty;
	int movePages;
	int clientID;
	vec2 otherCursors[16];
	bool activeCursors[16];
} editor;

editor editor_new(file* f, int clientID);

bool editor_update(editor* e, const net_socket* sockets, int socket_count);

void editor_process_packet(editor* e, char id, const char* packet);

void editor_render(editor* e, vec2 pos, vec2 size);

void editor_close(const editor* e, const net_socket* sockets, int socket_count);