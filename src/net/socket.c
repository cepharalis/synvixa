#include "socket.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#elifdef _WIN32
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

static WSADATA wsaData;
static bool init = false;
#else
#error Unsupported platform
#endif

bool socket_init() {
#if defined(__linux__) || defined(__APPLE__)
	return true;
#elifdef _WIN32
	if (!init) {
		const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			return false;
		}
		init = true;
	}
	return true;
#else
#error Unsupported platform
#endif
}

net_socket socket_open(const char* ip, const int port) {
	net_socket s;
	s.open = false;
#if defined(__linux__) || defined(__APPLE__)
	s.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s.socket == -1) {
		return s;
	}
	const int flags = fcntl(s.socket, F_GETFL);
	if (fcntl(s.socket, F_SETFL, flags | O_NONBLOCK) < 0) {
		return s;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);
	const int result = connect(s.socket, (struct sockaddr*) &addr, sizeof(addr));
	if (result < 0) {
		if (errno == EINPROGRESS) {
			fd_set set;
			FD_ZERO(&set);
			FD_SET(s.socket, &set);
			struct timeval time;
			time.tv_sec = 5;
			time.tv_usec = 0;
			if (select(s.socket + 1, nullptr, &set, nullptr, &time) < 0) {
				perror("select");
				return s;
			}
			int error = 0;
			socklen_t len = sizeof(error);
			getsockopt(s.socket, SOL_SOCKET, SO_ERROR, &error, &len);
			if (error != 0) {
				errno = error;
				perror("select");
				return s;
			}
		}
		else {
			perror("connect");
			printf("Error: %d\n", result);
			return s;
		}
	}
#elifdef _WIN32
	s.socket = INVALID_SOCKET;
	if (!socket_init()) {
		return s;
	}
	struct addrinfo hints;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	struct addrinfo* infoResult;
	char portStr[7];
	sprintf(portStr, "%d", port);
	if (getaddrinfo(ip, portStr, &hints, &infoResult) != 0) {
		return s;
	}
	s.socket = socket(infoResult->ai_family, infoResult->ai_socktype, infoResult->ai_protocol);
	if (s.socket == INVALID_SOCKET) {
		freeaddrinfo(infoResult);
		return s;
	}
	const int result = connect(s.socket, infoResult->ai_addr, (int) infoResult->ai_addrlen);
	if (result == SOCKET_ERROR) {
		closesocket(s.socket);
		return s;
	}
	freeaddrinfo(infoResult);
	if (result == INVALID_SOCKET) {
		return s;
	}
	u_long mode = 1;
	if (ioctlsocket(s.socket, FIONBIO, &mode) != 0) {
		return s;
	}
#else
#error Unsupported platform
#endif
	s.open = true;
	return s;
}

void socket_write(const net_socket* s, const void* data, const int len) {
	if (!s->open) {
		return;
	}
#if defined(__linux__) || defined(__APPLE__)
	ssize_t written = 0;
	while ((written += write(s->socket, data + written, len - written)) < len) {}
#elifdef _WIN32
	int written = 0;
	int result = 0;
	while (written != len) {
		result = send(s->socket, data + written, len - written, 0);
		if (result == SOCKET_ERROR) {
			closesocket(s->socket);
			return;
		}
		written += result;
	}
#else
#error Unsupported platform
#endif
}

int socket_read(const net_socket* s, void* data, const int len) {
	if (!s->open) {
		return -1;
	}
#if defined(__linux__) || defined(__APPLE__)
	const ssize_t l = read(s->socket, data, len);
	return l < 0 ? 0 : (int) l;
#elifdef _WIN32
	const int l = recv(s->socket, data, len, 0);
	return l < 0 ? 0 : l;
#else
#error Unsupported platform
#endif
}

void socket_close(net_socket* s) {
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
