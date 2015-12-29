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

#ifdef __linux__ 
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "commbus.h"
#include "socket.h"

#define TEST_HOST    	"192.168.133.22"
#define TEST_PORT	    5000
#define TEST_LEN    	1

int check_pattern(unsigned char *buf, unsigned char pattern, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (buf[i] != pattern)
			return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct sock_info_t sock;
	int ret;
	int i;
	int len;
	unsigned char *tx_buf;
	unsigned char *rx_buf;
	unsigned char pattern;

	if (argc == 2) {
		len = atoi(argv[1]);
	} else {
		len = TEST_LEN;
	}

	tx_buf = (unsigned char *)malloc(sizeof(unsigned char)*len);
	if (tx_buf == NULL) {
		debug_print("malloc failed!\n");
		return 1;        
	}

	rx_buf = (unsigned char *)malloc(sizeof(unsigned char)*len);
	if (rx_buf == NULL) {
		free(tx_buf);
		debug_print("malloc failed!\n");
		return 1;        
	}

	ret = socket_open(&sock, TYPE_TCP);
	if (ret != LIBCOMMBUS_SUCCESS) {
		debug_print("open failed!\n");
		getchar();
		return 1;
	}

	ret = socket_connect(&sock, TEST_HOST, TEST_PORT);
        if (ret != LIBCOMMBUS_SUCCESS) {
                debug_print("connect failed!\n");
                getchar();
                return 1;
        }
	printf("--Ready to Run Test--\n");

	pattern = 0;
	while (1) {
		memset(tx_buf, pattern, len);

		ret = socket_write(&sock, tx_buf, len);
		if (ret != len) {
			debug_print("socket_write: len=%d error!\n", ret);
		}

		ret = socket_read(&sock, rx_buf, len);
		if (ret != len) {
			debug_print("socket_read: len=%d error!\n", ret);
		}

		if (check_pattern(rx_buf, pattern, len) == 0) {
			pattern++;
		} else {
			for (i = 0; i < len; i++) {
				printf("(%d)TX: %x, RX: %x\n", i, tx_buf[i], rx_buf[i]);
			}

			while (1);
		}
		debug_print("RX:%x\n", rx_buf[0]);

#ifdef __linux__ 
		usleep(1000);
#else
		Sleep(1);
#endif
	}

	free(tx_buf);
	free(rx_buf);

	return 0;
} 
