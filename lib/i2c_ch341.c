#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "commbus.h"
#include "i2c.h"
#include "usb.h"

#define USBDEV_VID	0x1a86
#define USBDEV_PID	0x5512
#define USBDEV_IFACE	0

#define USBDEV_EPOUT	0x02
#define USBDEV_EPIN	0x82

#define CH341_I2C_LOW_SPEED 		0	// low speed - 20kHz
#define CH341_I2C_STANDARD_SPEED 	1	// standard speed - 100kHz
#define CH341_I2C_FAST_SPEED 		2	// fast speed - 400kHz
#define CH341_I2C_HIGH_SPEED 		3	// high speed - 750kHz

#define CH341_PACKET_LENGTH		32

#define	CH341_CMD_SET_OUTPUT		0xA1
#define CH341_CMD_IO_ADDR		0xA2
#define CH341_CMD_PRINT_OUT		0xA3
#define CH341_CMD_SPI_STREAM		0xA8
#define CH341_CMD_SIO_STREAM		0xA9
#define CH341_CMD_I2C_STREAM		0xAA
#define CH341_CMD_UIO_STREAM		0xAB

#define CH341_CMD_I2C_STM_STA		0x74
#define CH341_CMD_I2C_STM_STO		0x75
#define CH341_CMD_I2C_STM_OUT		0x80
#define CH341_CMD_I2C_STM_IN		0xC0
#define CH341_CMD_I2C_STM_MAX		(min(0x3F, CH341_PACKET_LENGTH))
#define CH341_CMD_I2C_STM_SET		0x60
#define CH341_CMD_I2C_STM_US		0x40
#define CH341_CMD_I2C_STM_MS		0x50
#define CH341_CMD_I2C_STM_DLY		0x0F
#define CH341_CMD_I2C_STM_END		0x00

#define CH341_RESP_OK			0x00
#define CH341_RESP_FAILED		0x01
#define CH341_RESP_BAD_MEMADDR		0x04
#define CH341_RESP_DATA_ERR		0x05
#define CH341_RESP_NOT_IMPLEMENTED	0x06
#define CH341_RESP_NACK			0x07
#define CH341_RESP_TIMEOUT		0x09

static unsigned char rx_buf[256];

static int ch341_xfer(unsigned char *buf, int len, int wr)
{
	int ret;

	ret = usb_bulk(USBDEV_EPOUT, buf, len);
	if (ret < 0)
		return ret;

	if (!wr) {
		memset(rx_buf, 0, 256);
		ret = usb_bulk(USBDEV_EPIN, rx_buf, 256);
		if (ret < 0)
			return ret;
	}

	return ret;
}

int i2c_open(int bus)
{
	int ret;
	unsigned char buf[3];

	buf[0] = CH341_CMD_I2C_STREAM;
	buf[1] = CH341_CMD_I2C_STM_SET | CH341_I2C_STANDARD_SPEED;
	buf[2] = CH341_CMD_I2C_STM_END;

	ret = usb_open(USBDEV_VID, USBDEV_PID, USBDEV_IFACE);
	if (ret != LIBCOMMBUS_SUCCESS)
		return -LIBCOMMBUS_ERROR_ACCESS;

	ret = ch341_xfer(buf, 3, 1);
	if (ret < 0)
		return ret;

	return LIBCOMMBUS_SUCCESS;
}

int i2c_read(int bus, unsigned short slave_addr, unsigned char reg_addr, 
		unsigned char *data, unsigned short len)
{
	int ret;
	int i;
	int l;
	unsigned char *buf;

	buf = (unsigned char *)malloc(sizeof(unsigned char)*(len+6));
	if (buf == NULL)
		return -LIBCOMMBUS_ERROR_MALLOC;

	buf[0] = CH341_CMD_I2C_STREAM;
	buf[1] = CH341_CMD_I2C_STM_STA;
	buf[2] = CH341_CMD_I2C_STM_OUT | 0x2;
	buf[3] = slave_addr << 1;
	buf[4] = reg_addr;

	ret = ch341_xfer(buf, 5, 1);
	if (ret < 0)
		goto out_read;

	buf[0] = CH341_CMD_I2C_STREAM;
	buf[1] = CH341_CMD_I2C_STM_STA;
	buf[2] = CH341_CMD_I2C_STM_OUT | 0x1;
	buf[3] = (slave_addr << 1) | 0x1;

	for (i = 0, l = len; l > 0; l--, i++)
		buf[i+4] = CH341_CMD_I2C_STM_IN | (l - 1);

	buf[len+4] = CH341_CMD_I2C_STM_STO;
	buf[len+5] = CH341_CMD_I2C_STM_END;

	ret = ch341_xfer(buf, len+5, 0);
	if (ret < 0)
		goto out_read;

	memcpy(data, rx_buf, ret);


out_read:
	if (buf) free(buf);
	return ret;
}

int i2c_write(int bus, unsigned short slave_addr, unsigned char reg_addr, 
		unsigned char *data, unsigned short len)
{
	int ret;
	unsigned char *buf;

	buf = (unsigned char *)malloc(sizeof(unsigned char)*(len+7));
	if (buf == NULL)
		return -LIBCOMMBUS_ERROR_MALLOC;

	buf[0] = CH341_CMD_I2C_STREAM;
	buf[1] = CH341_CMD_I2C_STM_STA;
	buf[2] = CH341_CMD_I2C_STM_OUT | (len + 2);
	buf[3] = slave_addr << 1;
	buf[4] = reg_addr;

	memcpy(&buf[5], data, len);

	buf[len+5] = CH341_CMD_I2C_STM_STO;
	buf[len+6] = CH341_CMD_I2C_STM_END;

	ret = ch341_xfer(buf, len+6, 1);
	if (ret < 0)
		goto out_write;

out_write:
	if (buf) free(buf);
	return ret;
}

int i2c_close(int bus)
{
	int ret;

	ret = usb_close(USBDEV_IFACE);
	if (ret != LIBCOMMBUS_SUCCESS)
		return -LIBCOMMBUS_ERROR_ACCESS;

	return LIBCOMMBUS_SUCCESS;
}
