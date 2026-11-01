#include "server_socket.h"

#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

server_socket server_socket_open(const int port) {
	server_socket s;
	s.open = false;
	s.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s.socket < 0) {
		return s;
	}
	const int flags = fcntl(s.socket, F_GETFL);
	if (fcntl(s.socket, F_SETFL, flags | O_NONBLOCK) < 0) {
		return s;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(s.socket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		return s;
	}
	if (listen(s.socket, 3) < 0) {
		return s;
	}
	s.open = true;
	return s;
}

net_socket server_socket_accept(const server_socket* s) {
	net_socket client;
	client.open = false;
	client.socket = -1;
	if (!s->open) {
		return client;
	}
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	client.socket = accept(s->socket, (struct sockaddr*) &addr, &len);
	if (client.socket < 0) {
		return client;
	}
	const int flags = fcntl(client.socket, F_GETFL);
	if (fcntl(client.socket, F_SETFL, flags | O_NONBLOCK) < 0) {
		return client;
	}
	client.open = true;
	return client;
}

void server_socket_close(server_socket* s) {
	if (!s->open) {
		return;
	}
	close(s->socket);
	s->open = false;
}
