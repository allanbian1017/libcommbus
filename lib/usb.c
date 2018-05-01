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
#include <stdint.h>
#include <libusb.h>
#include "commbus.h"
#include "usb.h"

static libusb_device_handle *cusb_fd = NULL;

int usb_open(uint16_t vid, uint16_t pid, int iface)
{
	int ret;

	ret = libusb_init(NULL);
	if (ret != LIBUSB_SUCCESS)
		return -LIBCOMMBUS_ERROR_ACCESS;

	cusb_fd = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if (cusb_fd == NULL)
		goto deinit;

	ret = libusb_kernel_driver_active(cusb_fd, iface);
	if (ret == 1) {
		ret = libusb_detach_kernel_driver(cusb_fd, iface);
		if (ret != LIBUSB_SUCCESS)
			goto close;
	}

	ret = libusb_claim_interface(cusb_fd, iface);
	if (ret != LIBUSB_SUCCESS)
		goto close;

	return LIBCOMMBUS_SUCCESS;

close:
	libusb_close(cusb_fd);
deinit:
	libusb_exit(NULL);
	return -LIBCOMMBUS_ERROR_ACCESS;
}

int usb_ctrl(unsigned char req_type, unsigned char req, uint32_t val, 
		uint32_t idx, unsigned char *data, uint16_t len)
{
	int ret;

	ret = libusb_control_transfer(cusb_fd, req_type, req, val, idx, data, len, 0);
	if (ret < 0)
		return -LIBCOMMBUS_ERROR_ACCESS;

	return ret;
}

int usb_bulk(unsigned char ep, unsigned char *data, int len)
{
	int act_len;
	int ret;

	if (len % 512 == 0) {
		ret = libusb_bulk_transfer(cusb_fd, ep, data, len - 1, &act_len, 0);
		if (ret != LIBUSB_SUCCESS)
			return -LIBCOMMBUS_ERROR_ACCESS;

		if (act_len != (len - 1))
			return act_len;
			
		ret = libusb_bulk_transfer(cusb_fd, ep, &data[act_len], 1, &act_len, 0);
                if (ret != LIBUSB_SUCCESS || act_len != 1)
                        return (len - 1);

		act_len = len;
	} else {
		ret = libusb_bulk_transfer(cusb_fd, ep, data, len, &act_len, 0);
                if (ret != LIBUSB_SUCCESS)
                        return -LIBCOMMBUS_ERROR_ACCESS;
	}

	return act_len;
}

int usb_close(int iface)
{
	int ret;

	ret = libusb_release_interface(cusb_fd, iface);
	if (ret != 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;

	ret = libusb_attach_kernel_driver(cusb_fd, iface);

	libusb_close(cusb_fd);
	libusb_exit(NULL);

	return LIBCOMMBUS_SUCCESS;
}
