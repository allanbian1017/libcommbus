#ifndef I2C_H
#define I2C_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
     I2C_BUS0 = 0,
     I2C_BUS1,
     I2C_BUS2,
     I2C_BUS_MAX
};

extern int i2c_open(int bus);
extern int i2c_read(int bus, unsigned short slave_addr, unsigned char reg_addr, 
                 unsigned char *data, unsigned short len);
extern int i2c_write(int bus, unsigned short slave_addr, unsigned char reg_addr, 
                 unsigned char *data, unsigned short len);
extern int i2c_close(int bus);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
