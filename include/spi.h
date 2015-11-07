#ifndef SPI_H
#define SPI_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
     SPI_BUS0 = 0,
     SPI_BUS1,
     SPI_BUS2,
     SPI_BUS_MAX
};

enum {
     SPI_CS0 = 0,
     SPI_CS1,
     SPI_CS_MAX
};

enum {
     SPI_MODE0 = 0,
     SPI_MODE1,
     SPI_MODE2,
     SPI_MODE3
};

extern int spi_open(int bus, int cs, int mode, unsigned int speed);
extern int spi_xfer(int bus, int cs, unsigned char *tx, unsigned char *rx, int len);
extern int spi_close(int bus, int cs);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
