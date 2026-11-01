#pragma once

#ifdef _WIN32
#include <winsock2.h>
#endif

typedef struct net_socket {
	bool open;
#if defined(__linux__) || defined(__APPLE__)
	int socket;
#elifdef _WIN32
	SOCKET socket;
#else
#error Unsupported platform
#endif
} net_socket;

bool socket_init();

net_socket socket_open(const char* ip, int port);

void socket_write(const net_socket* s, const void* data, int len);

int socket_read(const net_socket* s, void* data, int len);

void socket_close(net_socket* s);