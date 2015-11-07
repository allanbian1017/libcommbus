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
#include <inttypes.h>
#include "commbus.h"
#include "spi.h"

#define CONFIG_SPI_BUS  SPI_BUS1
#define CONFIG_SPI_CS   SPI_CS0

#define XFER_BUF_LEN	32

#define MAX_NAME_LEN    64
#define MAX_ID_LEN      10

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define CMD(_rdid, _wren, _rdsr, _write, _read)      \
        (&(struct spi_fram_cmd) {    \
         .rdid = (_rdid),                     \
         .wren = (_wren),                     \
         .rdsr = (_rdsr),                     \
         .write = (_write),                   \
         .read = (_read)                      \
         })

struct spi_fram_cmd {
	uint8_t rdid;
	uint8_t wren;
	uint8_t rdsr;
	uint8_t write;
	uint8_t read;
};

struct spi_fram_info {
	uint8_t name[MAX_NAME_LEN];
	int size;

	int id_len;
	uint8_t id[MAX_ID_LEN];

	struct spi_fram_cmd *cmd;
};

static struct spi_fram_info fram_info[] = {
	{"FM25V20", 256*1024, 9, {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0xc2, 0x25, 0x00},
		CMD(0x9f, 0x06, 0x05, 0x02, 0x0b)},
	{"MB85RS2MT", 256*1024, 4, {0x04, 0x7f, 0x28, 0x03},
		CMD(0x9f, 0x06, 0x05, 0x02, 0x0b)},
	{"FM25V02A", 32*1024, 9, {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0xc2, 0x22, 0x08},
		CMD(0x9f, 0x06, 0x05, 0x02, 0x0b)},
	{"MB85RS256B", 32*1024, 4, {0x04, 0x7f, 0x05, 0x09},
		CMD(0x9f, 0x06, 0x05, 0x02, 0x0b)},
	{ }
};

static struct spi_fram_info *spi_fram_probe(void)
{
	int ret;
	int match;
	int i;
	int j;
	uint8_t idcode[MAX_ID_LEN];
	uint8_t tx_buf[XFER_BUF_LEN];
	uint8_t rx_buf[XFER_BUF_LEN];
	struct spi_fram_info *curr;

	for (i = 0; i < ARRAY_SIZE(fram_info) - 1; i++) {
		match = 1;
		curr = &fram_info[i];

		memset(tx_buf, 0, XFER_BUF_LEN);
		memset(rx_buf, 0, XFER_BUF_LEN);
		tx_buf[0] = curr->cmd->rdid;

		ret = spi_xfer(CONFIG_SPI_BUS, CONFIG_SPI_CS, tx_buf, rx_buf, curr->id_len+1);
		if (ret != (curr->id_len+1))
			return NULL;

		memcpy(idcode, &rx_buf[1], curr->id_len);

		debug_print("ID:\n");
		for (j = 0; j < curr->id_len; j++) {
			debug_print("%x\n", idcode[j]);
			if (curr->id[j] != idcode[j]) {
				match = 0;
				break;
			}
		}

		if (match)
			goto found;
	}

	return NULL;

found:
	return curr;
}

int main(int argc, char *argv[])
{
	int ret;
	struct spi_fram_info *target;

	ret = spi_open(CONFIG_SPI_BUS, CONFIG_SPI_CS, SPI_MODE0, 1000000);
	if (ret != LIBCOMMBUS_SUCCESS) {
		printf("spi open failed\n");
		return 1;
	}

	target = spi_fram_probe();
	if (target) {
		printf("%s is found\n", target->name);
	} else {
		printf("Did not find any spi fram\n");
	}

	ret = spi_close(CONFIG_SPI_BUS, CONFIG_SPI_CS);
	if (ret != LIBCOMMBUS_SUCCESS) {
		printf("spi close failed\n");
		return 1;
	}

	return 0;
}
