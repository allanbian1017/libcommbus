#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "commbus.h"
#include "i2c.h"

#define CONFIG_I2C_BUS          I2C_BUS0
#define CONFIG_SLAVE_ADDR       0x50

#define CONFIG_I2C_EEPROM_WRITE
#define CONFIG_I2C_EEPROM_READ

static void print_hex(unsigned char *data, int len)
{
	int i ;
	for( i=0 ; i< len ; i++ )
	{
		if(( i!=0 )&&(i%8 == 0 )) printf( " " ) ;
		if(( i!=0 )&&(i%16 == 0 )) printf( "\n" ) ;
		printf( "%02x " , (int)data[i] ) ;
	}

	printf("\n");
}

static int eeprom_read(unsigned char addr, unsigned char offset,
		unsigned char *buf, unsigned short len)
{
	int ret;
	int s;
	int i;
        int l;
        int remain_len;

	remain_len = len;
        for (i = 0; i < len; i += 8) {
		s = offset + i;
                l = (remain_len >= 8) ? 8 : remain_len;

                ret = i2c_read(CONFIG_I2C_BUS, addr, s, &buf[i], l);
                if (ret < 0) {
                        printf("i2c read failed!\n");
                        return -1;
                }

                remain_len -= l;
        }


	return 0;

}

static int eeprom_write(unsigned char addr, unsigned char offset, 
		unsigned char *buf, unsigned short len)
{
	int ret;
	int s;
	int i;
	int l;
	int remain_len;

	remain_len = len;
	for (i = 0; i < len; i += 8) {
		s = offset + i;
		l = (remain_len >= 8) ? 8 : remain_len;

		ret = i2c_write(CONFIG_I2C_BUS, addr, s, &buf[i], l);
		if (ret < 0) {
			printf("i2c write failed!\n");
			return -1;
		}

		remain_len -= l;

		/* delay t_wr */
		usleep(6000);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	unsigned char buf[256];
	unsigned char key[32] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	ret = i2c_open(CONFIG_I2C_BUS);
	if (ret < 0) {
		printf("i2c init failed!\n");
		return 1;        
	}

	memset(buf, 0xcc, 256);

	buf[0] = 0x00;
	memcpy(&buf[1], key, 32);

#ifdef CONFIG_I2C_EEPROM_WRITE
	ret = eeprom_write(CONFIG_SLAVE_ADDR, 0, buf, 256);
	if (ret < 0) {
		printf("eeprom_write failed!\n");
		return 1;
	}
#endif

#ifdef CONFIG_I2C_EEPROM_READ
	ret = eeprom_read(CONFIG_SLAVE_ADDR, 0, buf, 256);
	if (ret < 0) {
		printf("eeprom_read failed!\n");
		return 1;
	}

	printf("EEPROM:\n");
	print_hex(buf, 256);
#endif

	i2c_close(CONFIG_I2C_BUS);
	return 0;
}
