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
#include "uart.h"

#define TEST_LEN    1

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

	ret = uart_open(COM1, 115200, PAR_NONE, DATBITS_8, STOPBITS_1);
	if (ret != LIBCOMMBUS_SUCCESS) {
		debug_print("open failed!\n");
		getchar();
		return 1;
	}
	printf("--Ready to Run Test--\n");

	pattern = 0;
	while (1) {
		memset(tx_buf, pattern, len);

		ret = uart_write(COM1, tx_buf, len);
		if (ret != len) {
			debug_print("uart_write: len=%d error!\n", ret);
		}

		ret = uart_read(COM1, rx_buf, len);
		if (ret != len) {
			debug_print("uart_read: len=%d error!\n", ret);
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
