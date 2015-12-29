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
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "commbus.h"
#include "i2c.h"

static int i2c_fd[I2C_BUS_MAX];

int i2c_open(int bus)
{
	char path[16];

	if (bus >= I2C_BUS_MAX || bus < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	sprintf(path, "/dev/i2c-%d", bus);

	i2c_fd[bus] = open(path, O_RDWR);
	if (i2c_fd[bus] < 0) {
		perror("open");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int i2c_read(int bus, unsigned short slave_addr, unsigned char reg_addr, 
		unsigned char *data, unsigned short len)
{
	int ret;
	struct i2c_rdwr_ioctl_data xfer;
	struct i2c_msg msg[2];

	if (bus >= I2C_BUS_MAX || bus < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	memset(&xfer, 0, sizeof(xfer));

	xfer.msgs = msg;
	xfer.nmsgs = 2;

	msg[0].addr = slave_addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg_addr;

	msg[1].addr = slave_addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = data;

	ret = ioctl(i2c_fd[bus], I2C_RDWR, &xfer);
	if (ret < 0) {
		perror("ioctl");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return ret;
}

int i2c_write(int bus, unsigned short slave_addr, unsigned char reg_addr, 
		unsigned char *data, unsigned short len)
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data xfer;
	struct i2c_msg msg[1];
	unsigned char *buf;

	if (bus >= I2C_BUS_MAX || bus < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	memset(&xfer, 0, sizeof(xfer));

	buf = (unsigned char *)malloc(len+1);
	if (!buf)
		return -LIBCOMMBUS_ERROR_MALLOC;

	xfer.msgs = msg;
	xfer.nmsgs = 1;

	buf[0] = reg_addr;
	memcpy(buf + 1, data, len);

	msg[0].addr = slave_addr;
	msg[0].flags = 0;
	msg[0].len = len + 1;
	msg[0].buf = buf;

	ret = ioctl(i2c_fd[bus], I2C_RDWR, &xfer);
	if (ret < 0) {
		perror("ioctl");
		free(buf);
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	free(buf);
	return ret;
}

int i2c_close(int bus)
{
	int ret;

	if (bus >= I2C_BUS_MAX || bus < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	ret = close(i2c_fd[bus]);
	if (ret != 0) {
		perror("close");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}
