#include "socket.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <sys/socket.h>

net_socket socket_open(const char* ip, const int port) {
	net_socket s;
	s.open = false;
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
			time.tv_sec = 1;
			time.tv_usec = 0;
			if (select(s.socket + 1, nullptr, &set, nullptr, &time) < 0) {
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
	s.open = true;
	return s;
}

void socket_write(const net_socket* s, const void* data, const int len) {
	if (!s->open) {
		return;
	}
	ssize_t written = 0;
	while ((written += write(s->socket, data + written, len - written)) < len) {}
}

int socket_read(const net_socket* s, void* data, const int len) {
	if (!s->open) {
		return -1;
	}
	const ssize_t l = read(s->socket, data, len);
	return l < 0 ? 0 : (int) l;
}

void socket_close(net_socket* s) {
	if (!s->open) {
		return;
	}
	close(s->socket);
	s->open = false;
}
