#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
     PAR_NONE = 0,
     PAR_ODD,
     PAR_EVEN
};

enum {
     DATBITS_6 = 0,
     DATBITS_7,
     DATBITS_8
};

enum {
     STOPBITS_1 = 0,
     STOPBITS_2
};

enum {
     COM0 = 0,
     COM1,
     COM2,
     COM3,
     COM4,
     COM5,
     COM6,
     COM7,
     COM_MAX
};

extern int uart_open(int com, int baudrate, int parity, int databits, int stopbits);
extern int uart_read(int com, unsigned char *data, int len);
extern int uart_write(int com, unsigned char *data, int len);
extern int uart_flush(int com);
extern int uart_close(int com);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
