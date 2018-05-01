#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commbus.h"
#include "uart.h"

#define TEST_LEN    1

void check_pattern(unsigned char *buf, unsigned char pattern, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (buf[i] != pattern) {
			printf("(%d)RX = %x Pattern = %x\n", i, buf[i], pattern);
		}
	}
}

int main(int argc, char *argv[])
{
	int ret;
	int len;
	unsigned char *buf = NULL;

	if (argc == 2) {
		len = atoi(argv[1]);
	} else {
		len = TEST_LEN;
	}

	buf = (unsigned char *)malloc(sizeof(unsigned char)*len);
	if (buf == NULL) {
		debug_print("malloc failed!\n");
		return 1;        
	}

	ret = uart_open(COM0, 115200, PAR_NONE, DATBITS_8, STOPBITS_1);
	if (ret != LIBCOMMBUS_SUCCESS) {
		debug_print("open failed!\n");
		return 1;
	}
	debug_print("--Ready to Run Test--\n");

	while (1) {
		ret = uart_read(COM0, buf, len);
		if (ret != len) {
			debug_print("uart_read: len=%d error!\n", ret);
		}

		check_pattern(buf, buf[0], len);

		ret = uart_write(COM0, buf, len);
		if (ret != len) {
			debug_print("uart_write: len=%d error!\n", ret);
		}

		debug_print("RX: %x\n", buf[0]);
	}

	free(buf);

	return 0;
}


