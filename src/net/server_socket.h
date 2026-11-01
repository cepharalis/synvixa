#pragma once

#include "socket.h"

typedef struct server_socket {
	bool open;
	int socket;
} server_socket;

server_socket server_socket_open(int port);

net_socket server_socket_accept(const server_socket* s);

void server_socket_close(server_socket* s);