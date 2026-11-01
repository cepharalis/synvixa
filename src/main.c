// This comment was written with synvixa
// YAY this was written with synvixa client and this was written by synvixa server
// And this was written by a second client

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "net/server_socket.h"
#include "net/socket.h"
#include "plugin/plugins.h"
#include "ui/editor.h"
#include "ui/term.h"
#include "util/string_util.h"

#define PACKET_DISCONNECT 0
#define PORT 25565

static volatile sig_atomic_t running = 1;

static const char* HEADER = "SYNV0001";

static void handle_sigint(int) {
	running = 0;
}

int main(const int argc, char** argv) {
	const bool client = argc == 3 && strcmp(argv[1], "--ip") == 0;
	if (argc != 2 && !client) {
		printf("Usage:\n\t%s <filename>\n\t%s --ip <ip_address>\n", argv[0], argv[0]);
		return 1;
	}
	signal(SIGINT, handle_sigint);
#if defined(__linux__) || defined(__APPLE__)
	signal(SIGPIPE, SIG_IGN);
#endif
	plugins_load();
	file f;
	net_socket client_socket = {};
	server_socket server_socket = {};
	net_socket clients[16] = {};
	for (int i = 0; i < 16; i++) {
		clients[i].open = false;
	}
	int client_count = 0;
	char buffer[1024] = {};
	if (client) {
		client_socket = socket_open(argv[2], PORT);
		if (!client_socket.open) {
			printf("Failed to connect to server\n");
			return 1;
		}
		int read = 0;
		while ((read += socket_read(&client_socket, buffer + read, 9 - read)) < 9) {}
		if (strncmp(buffer, HEADER, 8) != 0) {
			printf("Server sent invalid header");
			return 1;
		}
		client_count = buffer[8]; // NOLINT(*-str34-c)
		if (client_count == -1) {
			printf("Server busy, only 16 clients are supported");
			return 1;
		}
		f = file_receive(&client_socket);
	}
	else {
		server_socket = server_socket_open(PORT);
		f = file_read(argv[1]);
		if (f.lines.size == 0) {
			char_vector line;
			char_vector_init(&line, 0);
			char_vector_vector_push(&f.lines, line);
		}
	}
	term_init();
	const plugin_vector* plugins = plugins_get();
	f.language = nullptr;
	for (int i = 0; i < plugins->size; i++) {
		const language_vector* languages = &plugins->data[i].languages;
		for (int j = 0; j < languages->size; j++) {
			language* language = &languages->data[j];
			char** extensions = language->extensions;
			for (int k = 0; k < language->extension_count; k++) {
				if (string_ends_with(f.path, extensions[k])) {
					f.language = language;
					break;
				}
			}
		}
	}
	editor e = editor_new(&f, client ? client_count : -1);
	bool render = true;
	vec2 termSize = term_get_size();
	while (running) {
		if (client) {
			render |= editor_update(&e, &client_socket, 1);
		}
		else {
			render |= editor_update(&e, clients, 16);
		}
		const vec2 newSize = term_get_size();
		if (termSize.x != newSize.x || termSize.y != newSize.y) {
			termSize = newSize;
			render = true;
		}
		if (render) {
			term_clear(TERM_DEFAULT);
			editor_render(&e, (vec2){.x = 0, .y = 0}, term_get_size());
			term_render();
			render = false;
		}
		if (!client && server_socket.open) {
			net_socket c;
			while ((c = server_socket_accept(&server_socket)).open) {
				socket_write(&c, HEADER, 8);
				int index = -1;
				for (int i = 0; i < 16; i++) {
					if (!clients[i].open) {
						clients[i] = c;
						index = i;
						break;
					}
				}
				if (index == -1) {
					char d = -1;
					socket_write(&c, &d, 1);
				}
				else {
					char d = (char) index;
					socket_write(&c, &d, 1);
					file_send(&f, &c);
				}
				for (int i = 0; i < 17; i++) {
					if (i < 16 && !e.activeCursors[i]) {
						continue;
					}
					vec2 pos = i == 16 ? e.cursor : e.otherCursors[i];
					char packet[17];
					packet[0] = EDITOR_PACKET_SET_CURSOR;
					constexpr uint32_t length = 12;
					memcpy(packet + 1, &length, 4);
					memcpy(packet + 5, &pos, 8);
					int clientID = i == 16 ? -1 : i;
					memcpy(packet + 13, &clientID, 4);
					for (int j = 0; j < 16; j++) {
						socket_write(&clients[j], packet, 17);
					}
				}
			}
			for (int i = 0; i < 16; i++) {
				if (!clients[i].open) {
					continue;
				}
				int read;
				char id;
				while ((read = socket_read(&clients[i], &id, 1)) > 0) {
					if (id == PACKET_DISCONNECT) {
						socket_close(&clients[i]);
						break;
					}
					uint32_t length;
					read = 0;
					while ((read = socket_read(&clients[i], &length + read, 4 - read)) < 4) {}
					char* packet = malloc(length);
					read = 0;
					while ((read = socket_read(&clients[i], packet + read, (int) length - read)) < length) {}
					for (int j = 0; j < 16; j++) {
						if (i == j || !clients[j].open) {
							continue;
						}
						socket_write(&clients[j], &id, 1);
						socket_write(&clients[j], &length, 4);
						socket_write(&clients[j], packet, (int) length);
					}
					if (id >= EDITOR_PACKET_BASE && id <= EDITOR_PACKET_END) {
						editor_process_packet(&e, id, packet);
						render |= true;
					}
					free(packet);
				}
			}
		}
		else if (client) {
			int read;
			char id;
			while ((read = socket_read(&client_socket, &id, 1)) > 0) {
				if (id == PACKET_DISCONNECT) {
					socket_close(&client_socket);
					goto stop;
				}
				uint32_t length;
				read = 0;
				while ((read = socket_read(&client_socket, &length + read, 4 - read)) < 4) {}
				char* packet = malloc(length);
				read = 0;
				while ((read = socket_read(&client_socket, packet + read, (int) length - read)) < length) {}
				if (id >= EDITOR_PACKET_BASE && id <= EDITOR_PACKET_END) {
					editor_process_packet(&e, id, packet);
					render |= true;
				}
				free(packet);
			}
		}
		usleep(50000);
	}
stop:
	if (client) {
		term_destroy();
		if (client_socket.open) {
			editor_close(&e, &client_socket, 1);
			char id = PACKET_DISCONNECT;
			socket_write(&client_socket, &id, 1);
			socket_close(&client_socket);
		}
		else {
			printf("Server disconnected\n");
		}
	}
	else {
		char id = PACKET_DISCONNECT;
		for (int i = 0; i < 16; i++) {
			if (clients[i].open) {
				socket_write(&clients[i], &id, 1);
				socket_close(&clients[i]);
			}
		}
		server_socket_close(&server_socket);
		if (f.modified) {
			vec2 size = term_get_size();
			term_clear_line(size.y - 1);
			term_set_cursor((vec2){.x = 0, .y = size.y - 1});
			term_write_s("Save modifications [Y/n]?", TERM_BOLD);
			term_render();
			int c;
			while ((c = term_read()) == -1) {}
			if (c != 'n' && c != 'N') {
				file_write(&f);
			}
		}
		term_destroy();
	}
	plugins_unload();
	return 0;
}