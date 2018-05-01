#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "commbus.h"
#include "spi.h"

static int spi_fd[SPI_BUS_MAX][SPI_CS_MAX];

int spi_open(int bus, int cs, int mode, unsigned int speed)
{
	char path[16];
	int ret;
	unsigned char xfer_bits;
	unsigned char xfer_mode;

	if (bus >= SPI_BUS_MAX || bus < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	if (cs >= SPI_CS_MAX || cs < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	switch (mode) {
		case SPI_MODE0:
			xfer_mode = SPI_MODE_0;
			break;         
		case SPI_MODE1:
			xfer_mode = SPI_MODE_1;
			break;
		case SPI_MODE2:
			xfer_mode = SPI_MODE_2;
			break;
		case SPI_MODE3:
			xfer_mode = SPI_MODE_3;
			break;
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}

	/* Only support 8-bit data mode */
	xfer_bits = 8;

	sprintf(path, "/dev/spidev%d.%d", bus, cs);

	spi_fd[bus][cs] = open(path, O_RDWR);
	if (spi_fd[bus][cs] < 0) {
		perror("open");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	ret = ioctl(spi_fd[bus][cs], SPI_IOC_WR_MODE, &xfer_mode);
	if (ret != 0) {
		close(spi_fd[bus][cs]);
		perror("ioctl");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	ret = ioctl(spi_fd[bus][cs], SPI_IOC_WR_BITS_PER_WORD, &xfer_bits);
	if (ret != 0) {
		close(spi_fd[bus][cs]);
		perror("ioctl");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	ret = ioctl(spi_fd[bus][cs], SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret != 0) {
		close(spi_fd[bus][cs]);
		perror("ioctl");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}

int spi_xfer(int bus, int cs, unsigned char *tx, unsigned char *rx, int len)
{
	int ret;
	struct spi_ioc_transfer xfer;

	if (bus >= SPI_BUS_MAX || bus < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	if (cs >= SPI_CS_MAX || cs < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	memset((void *)&xfer, 0, sizeof(xfer));

	xfer.tx_buf = (unsigned long)tx;
	xfer.rx_buf = (unsigned long)rx;
	xfer.len = len;

	ret = ioctl(spi_fd[bus][cs], SPI_IOC_MESSAGE(1), &xfer);
	if (ret < 0) {
		perror("ioctl");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return ret;
}

int spi_close(int bus, int cs)
{
	int ret;

	if (bus >= SPI_BUS_MAX || bus < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	if (cs >= SPI_CS_MAX || cs < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	ret = close(spi_fd[bus][cs]);
	if (ret != 0) {
		perror("close");
		return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return LIBCOMMBUS_SUCCESS;
}
