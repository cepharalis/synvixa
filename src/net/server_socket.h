#pragma once

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "socket.h"

typedef struct server_socket {
	bool open;
#if defined(__linux__) || defined(__APPLE__)
	int socket;
#elifdef _WIN32
	SOCKET socket;
#else
#error Unsupported platform
#endif
} server_socket;

server_socket server_socket_open(int port);

net_socket server_socket_accept(const server_socket* s);

void server_socket_close(server_socket* s);