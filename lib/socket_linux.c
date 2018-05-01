#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "commbus.h"
#include "socket.h"

int socket_open(struct sock_info_t *sock, int type)
{
	int sock_type;
	int sock_domain;

	memset(&sock->addr, 0, sizeof(sock->addr));

	switch (type) {
		case TYPE_TCP:
			sock_domain = AF_INET;
			sock_type = SOCK_STREAM;
			break;

		default:
			return LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	sock->fd = socket(sock_domain, sock_type, 0);
	if (sock->fd == -1)
		return -LIBCOMMBUS_ERROR_ACCESS;

	sock->addr.sin_family = sock_domain;

	return LIBCOMMBUS_SUCCESS;
}

int socket_bind(struct sock_info_t *sock, int port)
{
	int ret;

	sock->addr.sin_port = htons(port);
	sock->addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sock->fd, (struct sockaddr *)&sock->addr, sizeof(sock->addr));
	if (ret != 0) {
		perror("bind");
		close(sock->fd);
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int socket_listen(struct sock_info_t *sock)
{
	int ret;

	ret = listen(sock->fd, SOMAXCONN);
	if (ret != 0) {
		perror("listen");
		close(sock->fd);
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int socket_accept(struct sock_info_t *sock_listen, struct sock_info_t *sock_conn)
{
	sock_conn->fd = accept(sock_listen->fd, (struct sockaddr *)NULL, NULL);
	if (sock_conn->fd == -1) {
		perror("accept");
		close(sock_listen->fd);
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int socket_connect(struct sock_info_t *sock, char *host, int port)
{
	int ret;

	sock->addr.sin_port = htons(port);
	ret = inet_pton(sock->addr.sin_family, host, &sock->addr.sin_addr);
	if (ret != 1) {
		perror("inet_pton");
		close(sock->fd);
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	ret = connect(sock->fd, (struct sockaddr *)&sock->addr, sizeof(sock->addr));
	if (ret != 0) {
		perror("connect");
		close(sock->fd);
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;	
}

int socket_read(struct sock_info_t *sock, unsigned char *data, int len)
{
	int recv;
	int remain;
	int n;

	recv = 0;
	remain = len;
	do {
		n = read(sock->fd, &data[recv], remain);
		if (n < 0) {
			return recv;
		}

		recv += n;
		remain = len - recv;
	} while (remain != 0);

	return recv;
}

int socket_write(struct sock_info_t *sock, unsigned char *data, int len)
{
	int sent;
	int remain;
	int n;

	sent = 0;
	remain = len;
	do {
		n = write(sock->fd, &data[sent], remain);
		if (n < 0) {
			return sent;
		}

		sent += n;
		remain = len - sent;
	} while (remain != 0);

	return sent;
}

int socket_close(struct sock_info_t *sock)
{
	int ret;

	ret = close(sock->fd);
	if (ret != 0) {
		perror("close");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}
