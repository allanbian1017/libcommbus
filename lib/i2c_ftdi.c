#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpsse.h>
#include "commbus.h"
#include "i2c.h"

struct mpsse_context *i2c_fd = NULL;

int i2c_open(int bus)
{
	i2c_fd = MPSSE(I2C, FOUR_HUNDRED_KHZ, MSB);
	if (i2c_fd == NULL)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	if (i2c_fd->open == 0) 
		return -LIBCOMMBUS_ERROR_ACCESS;

	return LIBCOMMBUS_SUCCESS;
}

int i2c_read(int bus, unsigned short slave_addr, unsigned char reg_addr,
		unsigned char *data, unsigned short len)
{
	int ret;
	int recv = 0;
	unsigned char *ptr = NULL;
	unsigned char buf[2];

	ret = Start(i2c_fd);
	if (ret != MPSSE_OK)
		return -LIBCOMMBUS_ERROR_ACCESS;

	buf[0] = (slave_addr & 0x7f) << 1;
	buf[1] = reg_addr;

	ret = Write(i2c_fd, buf, 2);
	if (ret != MPSSE_OK)
		return -LIBCOMMBUS_ERROR_ACCESS;

	if (GetAck(i2c_fd) == NACK)
		return -LIBCOMMBUS_ERROR_ACCESS;

	ret = Start(i2c_fd);
	if (ret != MPSSE_OK)
		return -LIBCOMMBUS_ERROR_ACCESS;

	buf[0] = (slave_addr & 0x7f) << 1 | 0x01;
	ret = Write(i2c_fd, buf, 1);
	if (ret != MPSSE_OK)
		return -LIBCOMMBUS_ERROR_ACCESS;

	if (GetAck(i2c_fd) == NACK)
		return -LIBCOMMBUS_ERROR_ACCESS;

	ptr = Read(i2c_fd, len);
	if (ptr) {
		memcpy(data, ptr, len);
		recv = len;

		free(ptr);
	}

	SendNacks(i2c_fd);

	/* Read in one dummy byte, with a NACK */
	Read(i2c_fd, 1);

	ret = Stop(i2c_fd);
	if (ret != MPSSE_OK)
		return -LIBCOMMBUS_ERROR_ACCESS;

	return recv;
}

int i2c_write(int bus, unsigned short slave_addr, unsigned char reg_addr,
		unsigned char *data, unsigned short len)
{
	int ret;
        int sent = 0;
        unsigned char *buf = NULL;

	buf = (unsigned char *)malloc(sizeof(unsigned char)*len);
	if (buf == NULL)
		return -LIBCOMMBUS_ERROR_MALLOC;

        ret = Start(i2c_fd);
        if (ret != MPSSE_OK)
                return -LIBCOMMBUS_ERROR_ACCESS;

        buf[0] = (slave_addr & 0x7f) << 1;
        buf[1] = reg_addr;
	memcpy(&buf[2], data, len);

        ret = Write(i2c_fd, buf, len+2);
        if (ret != MPSSE_OK)
                return -LIBCOMMBUS_ERROR_ACCESS;

        if (GetAck(i2c_fd) == NACK)
                return -LIBCOMMBUS_ERROR_ACCESS;

	ret = Stop(i2c_fd);
        if (ret != MPSSE_OK)
                return -LIBCOMMBUS_ERROR_ACCESS;

	sent = len;

	return sent;

}

int i2c_close(int bus)
{
	Close(i2c_fd);

	return LIBCOMMBUS_SUCCESS;
}

