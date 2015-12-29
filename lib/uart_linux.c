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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include "commbus.h"
#include "uart.h"

static int uart_fd[COM_MAX];

int uart_open(int com, int baudrate, int parity, int databits, int stopbits)
{
	struct termios setting;
	char path[32];
	int ret;
	int br;
	int par;
	int dbits;
	int sbits;

	if (com >= COM_MAX || com < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	switch (baudrate) {
		case 50:
			br = B50;
			break;
		case 75: 
			br = B75;
			break;
		case 110: 
			br = B110;
			break;
		case 134: 
			br = B134;
			break;
		case 150: 
			br = B150;
			break;
		case 200: 
			br = B200;
			break;
		case 300: 
			br = B300;
			break;
		case 600: 
			br = B600;
			break;
		case 1200: 
			br = B1200;
			break;
		case 1800: 
			br = B1800;
			break;
		case 2400: 
			br = B2400;
			break;
		case 4800: 
			br = B4800;
			break;
		case 9600: 
			br = B9600;
			break;
		case 19200: 
			br = B19200;
			break;
		case 38400: 
			br = B38400;
			break;
		case 57600: 
			br = B57600;
			break;
		case 115200: 
			br = B115200;
			break;
		case 230400: 
			br = B230400;
			break;
		case 460800: 
			br = B460800;
			break;
		case 500000: 
			br = B500000;
			break;
		case 576000: 
			br = B576000;
			break;
		case 921600: 
			br = B921600;
			break;
		case 1000000: 
			br = B1000000;
			break;
		default: 
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	switch (parity) {
		case PAR_NONE:
			par = 0;
			break;         
		case PAR_ODD:
			par = PARENB | PARODD;
			break;         
		case PAR_EVEN:
			par = PARENB;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	switch (databits) {
		case DATBITS_6:
			dbits = CS6;
			break;         
		case DATBITS_7:
			dbits = CS7;
			break;         
		case DATBITS_8:
			dbits = CS8;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	switch (stopbits) {
		case STOPBITS_1:
			sbits = 0;
			break;         
		case STOPBITS_2:
			sbits = CSTOPB;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	sprintf(path, "/dev/ttyS%d", com);

	uart_fd[com] = open(path, O_RDWR | O_NOCTTY);
	if (uart_fd[com] == -1) {
		perror("open");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	memset(&setting, 0, sizeof(setting));

	setting.c_cflag = br | dbits | sbits | par;
	setting.c_cflag |= CLOCAL | CREAD;
	setting.c_cflag &= ~CRTSCTS;
	setting.c_iflag = IGNPAR;

	/* blocking read/write operation */
	setting.c_cc[VMIN] = 1;
	setting.c_cc[VTIME] = 0;

	ret = tcflush(uart_fd[com], TCIOFLUSH);

	ret = tcsetattr(uart_fd[com], TCSANOW, &setting);
	if (ret != 0) {
		close(uart_fd[com]);
		perror("tcsetattr");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}


int uart_read(int com, unsigned char *data, int len)
{
	int recv;
	int remain;
	int n;

	if (com >= COM_MAX || com < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	recv = 0;
	remain = len;
	do {
		n = read(uart_fd[com], &data[recv], remain);
		if (n < 0) {
			return recv;
		}

		recv += n;
		remain = len - recv;
	} while (remain != 0);

	return recv;
}

int uart_write(int com, unsigned char *data, int len)
{
	int sent;
	int remain;
	int n;

	if (com >= COM_MAX || com < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	sent = 0;
	remain = len;
	do {
		n = write(uart_fd[com], &data[sent], remain);
		if (n < 0) {
			return sent;
		}

		sent += n;
		remain = len - sent;
	} while (remain != 0);

	return sent;
}

int uart_flush(int com)
{
	int ret;

	if (com > (COM_MAX-1) || com < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	ret = tcflush(uart_fd[com], TCIOFLUSH);
	if (ret != 0) {
		perror("tcflush");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int uart_close(int com)
{
	int ret;

	if (com >= COM_MAX || com < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	ret = close(uart_fd[com]);
	if (ret != 0) {
		perror("close");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

