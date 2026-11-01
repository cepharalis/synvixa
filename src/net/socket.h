#pragma once

typedef struct net_socket {
	bool open;
	int socket;
} net_socket;

net_socket socket_open(const char* ip, int port);

void socket_write(const net_socket* s, const void* data, int len);

int socket_read(const net_socket* s, void* data, int len);

void socket_close(net_socket* s);