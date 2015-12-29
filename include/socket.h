#ifndef SOCKET_H
#define SOCKET_H

#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sock_info_t {
#ifdef __linux__
	int fd;
	struct sockaddr_in addr;
#else
	WSADATA wsa_data;
	SOCKET fd;
	struct addrinfo addr;
#endif
};

enum {
	TYPE_TCP,
	TYPE_UDP,
	TYPE_UDS
};

extern int socket_open(struct sock_info_t *sock, int type);
extern int socket_bind(struct sock_info_t *sock, int port);
extern int socket_listen(struct sock_info_t *sock);
extern int socket_accept(struct sock_info_t *sock_listen, struct sock_info_t *sock_conn);
extern int socket_connect(struct sock_info_t *sock, char *host, int port);
extern int socket_read(struct sock_info_t *sock, unsigned char *data, int len);
extern int socket_write(struct sock_info_t *sock, unsigned char *data, int len);
extern int socket_close(struct sock_info_t *sock);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
