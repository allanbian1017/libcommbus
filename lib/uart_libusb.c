#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "commbus.h"
#include "usb.h"
#include "uart.h"

#define USBDEV_VID						0x0525
#define USBDEV_PID						0xa4a7
#define USBDEV_IFACE	  				0x1

#define USBDEV_EPOUT	  				0x02
#define USBDEV_EPIN	      				0x81

#define SET_LINE_REQUEST_TYPE           0x21
#define SET_LINE_REQUEST                0x20

enum {
	STOP_BIT_1 = 0,
	STOP_BIT_1_5,
	STOP_BIT_2
};

enum {
	PARITY_NONE = 0,
	PARITY_ODD,
	PARITY_EVEN
};

struct line_code {
	unsigned char baudrate[4];
	unsigned char stop;
	unsigned char parity;
	unsigned char data;
};


int uart_open(int com, int baudrate, int parity, int databits, int stopbits)
{
	int ret;
	struct line_code setting;

	setting.baudrate[0] = baudrate & 0xff;
	setting.baudrate[1] = (baudrate >> 8) & 0xff;
	setting.baudrate[2] = (baudrate >> 16) & 0xff;
	setting.baudrate[3] = (baudrate >> 24) & 0xff;

	switch (parity) {
		case PAR_NONE:
			setting.parity = PARITY_NONE;
			break;         
		case PAR_ODD:
			setting.parity = PARITY_ODD;
			break;         
		case PAR_EVEN:
			setting.parity = PARITY_EVEN;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	switch (databits) {
		case DATBITS_6:
			setting.data = 6;
			break;         
		case DATBITS_7:
			setting.data = 7;
			break;         
		case DATBITS_8:
			setting.data = 8;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	switch (stopbits) {
		case STOPBITS_1:
			setting.stop = STOP_BIT_1;
			break;         
		case STOPBITS_2:
			setting.stop = STOP_BIT_2;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	ret = usb_open(USBDEV_VID, USBDEV_PID, USBDEV_IFACE);
	if (ret != LIBCOMMBUS_SUCCESS)
		return -LIBCOMMBUS_ERROR_ACCESS;

	ret = usb_ctrl(SET_LINE_REQUEST_TYPE, SET_LINE_REQUEST, 0, 0, (unsigned char *)&setting, sizeof(setting));
	if (ret != sizeof(setting))
		return -LIBCOMMBUS_ERROR_ACCESS;

	return LIBCOMMBUS_SUCCESS;
}

int uart_read(int com, unsigned char *data, int len)
{
	int recv;
	int remain;
	int n;

	recv = 0;
	remain = len;
	do {
		n = usb_bulk(USBDEV_EPIN, &data[len-remain], remain);
		if (n < 0) {
			return recv;
		}

		recv += n;
		remain -= n;
	} while (remain > 0);

	return recv;
}

int uart_write(int com, unsigned char *data, int len)
{
	int sent;
	int remain;
	int n;

	sent = 0;
	remain = len;
	do {
		n = usb_bulk(USBDEV_EPOUT, &data[sent], remain);
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
	return LIBCOMMBUS_SUCCESS;
}

int uart_close(int com)
{
	int ret;

	ret = usb_close(USBDEV_IFACE);
	if (ret != LIBCOMMBUS_SUCCESS)
		return -LIBCOMMBUS_ERROR_ACCESS;

	return LIBCOMMBUS_SUCCESS;
}
