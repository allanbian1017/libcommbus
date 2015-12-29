/***************************************************************************
 *   Copyright (C) 2015 by Tse-Lun Bien                                    *
 *   allanbian@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "commbus.h"
#include "socket.h"

int socket_open(struct sock_info_t *sock, int type)
{
	int sock_type;
	int sock_domain;
	int sock_proto;
	int ret;

	ret = WSAStartup(MAKEWORD(2,2), &sock->wsa_data);
	if (LOBYTE(sock->wsa_data.wVersion) != 2 || HIBYTE(sock->wsa_data.wVersion) != 2) {
		WSACleanup();
		return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	memset(&sock->addr, 0, sizeof(sock->addr));

	switch (type) {
		case TYPE_TCP:
			sock_domain = AF_INET;
			sock_type = SOCK_STREAM;
			sock_proto = IPPROTO_TCP;
			break;

		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	sock->fd = socket(sock_domain, sock_type, sock_proto);
	if (sock->fd == INVALID_SOCKET) {
		ret = -LIBCOMMBUS_ERROR_ACCESS;
		goto out_init;
	}

	sock->addr.ai_family = sock_domain;
	sock->addr.ai_socktype = sock_type;
	sock->addr.ai_protocol = sock_proto;
	sock->addr.ai_flags = AI_PASSIVE;

	return LIBCOMMBUS_SUCCESS;

out_init:
	WSACleanup();
	return ret;
}

int socket_bind(struct sock_info_t *sock, int port)
{
	struct addrinfo *sock_addr = NULL;
	char sock_port[32];
	int ret;

	sprintf(sock_port, "%d", port);

	ret = getaddrinfo(NULL, sock_port, &sock->addr, &sock_addr);
	if (ret != 0) {
		closesocket(sock->fd);
		WSACleanup();
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	ret = bind(sock->fd, sock_addr->ai_addr, sock_addr->ai_addrlen);
	if (ret == SOCKET_ERROR) {
		freeaddrinfo(sock_addr);
		closesocket(sock->fd);
		WSACleanup();
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	freeaddrinfo(sock_addr);

	return LIBCOMMBUS_SUCCESS;
}

int socket_listen(struct sock_info_t *sock)
{
	int ret;

	ret = listen(sock->fd, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		closesocket(sock->fd);
		WSACleanup();
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int socket_accept(struct sock_info_t *sock_listen, struct sock_info_t *sock_conn)
{
	int ret;

	sock_conn->fd = accept(sock_listen->fd, NULL, NULL);
	if (sock_conn->fd == INVALID_SOCKET) {
		closesocket(sock_listen->fd);
		WSACleanup();
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int socket_connect(struct sock_info_t *sock, char *host, int port)
{
	struct addrinfo *sock_addr = NULL;
        char sock_port[32];
        int ret;

        sprintf(sock_port, "%d", port);

	ret = getaddrinfo(host, sock_port, &sock->addr, &sock_addr);
	if (ret != 0) {
		closesocket(sock->fd);
                WSACleanup();
                return -LIBCOMMBUS_ERROR_ACCESS;
	}

	ret = connect(sock->fd, sock_addr->ai_addr, sock_addr->ai_addrlen);
	if (ret == SOCKET_ERROR) {
		freeaddrinfo(sock_addr);
		closesocket(sock->fd);
                WSACleanup();
                return -LIBCOMMBUS_ERROR_ACCESS;
	}

	freeaddrinfo(sock_addr);

	if (sock->fd == INVALID_SOCKET) {
		closesocket(sock->fd);
                WSACleanup();
                return -LIBCOMMBUS_ERROR_ACCESS;
	}	

	return LIBCOMMBUS_SUCCESS;
}

int socket_read(struct sock_info_t *sock, unsigned char *data, int len)
{
	int recv_len;
	int remain;
	int n;

	recv_len = 0;
	remain = len;
	do {
		n = recv(sock->fd, &data[recv_len], remain, 0);
		if (n < 0) {
			return recv_len;
		}

		recv_len += n;
		remain = len - recv_len;
	} while (remain != 0);

	return recv_len;
}

int socket_write(struct sock_info_t *sock, unsigned char *data, int len)
{
	int sent_len;
	int remain;
	int n;

	sent_len = 0;
	remain = len;
	do {
		n = send(sock->fd, &data[sent_len], remain, 0);
		if (n < 0) {
			return sent_len;
		}

		sent_len += n;
		remain = len - sent_len;
	} while (remain != 0);

	return sent_len;
}

int socket_close(struct sock_info_t *sock)
{
	int ret;

	ret = shutdown(sock->fd, SD_BOTH);
	if (ret == SOCKET_ERROR) {
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	ret = closesocket(sock->fd);
	if (ret == SOCKET_ERROR) {
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	WSACleanup();

	return LIBCOMMBUS_SUCCESS;
}
