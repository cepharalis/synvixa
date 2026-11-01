#include "server_socket.h"

#include <stdio.h>
#include <unistd.h>
#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#elifdef _WIN32
#include <ws2tcpip.h>
#else
#error Unsupported platform
#endif

server_socket server_socket_open(const int port) {
	server_socket s;
	s.open = false;
#if defined(__linux__) || defined(__APPLE__)
	s.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s.socket < 0) {
		return s;
	}
	const int flags = fcntl(s.socket, F_GETFL);
	if (fcntl(s.socket, F_SETFL, flags | O_NONBLOCK) < 0) {
		return s;
	}
	constexpr int opt = 1;
	setsockopt(s.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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
#elifdef _WIN32
	s.socket = INVALID_SOCKET;
	if (!socket_init()) {
		return s;
	}
	struct addrinfo hints;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo* infoResult;
	char portStr[7];
	sprintf(portStr, "%d", port);
	if (getaddrinfo(nullptr, portStr, &hints, &infoResult) != 0) {
		return s;
	}
	s.socket = socket(infoResult->ai_family, infoResult->ai_socktype, infoResult->ai_protocol);
	if (s.socket == INVALID_SOCKET) {
		freeaddrinfo(infoResult);
		return s;
	}
	if (bind(s.socket, infoResult->ai_addr, (int) infoResult->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(infoResult);
		return s;
	}
	freeaddrinfo(infoResult);
	u_long mode = 1;
	if (ioctlsocket(s.socket, FIONBIO, &mode) != 0) {
		closesocket(s.socket);
		return s;
	}
	if (listen(s.socket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(s.socket);
		return s;
	}
#elifdef _WIN32
	s.socket = INVALID_SOCKET;
#else
#error Unsupported platform
#endif
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
#if defined(__linux__) || defined(__APPLE__)
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
#elifdef _WIN32
	client.socket = accept(s->socket, nullptr, nullptr);
	if (client.socket == INVALID_SOCKET) {
		return client;
	}
#else
#error Unsupported platform
#endif
	client.open = true;
	return client;
}

void server_socket_close(server_socket* s) {
	if (!s->open) {
		return;
	}
#if defined(__linux__) || defined(__APPLE__)
	close(s->socket);
#elifdef _WIN32
	closesocket(s->socket);
#else
#error Unsupported platform
#endif
	s->open = false;
}
