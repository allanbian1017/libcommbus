CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
CFLAGS = -Wall -Iinclude/
CFLAGS += -I/usr/local/include/libusb-1.0/
CPPFLAGS = -Wall -Iinclude/
LIBS = -lcommbus
LDFLAGS = 
LIB_OBJS = uart_linux.o spi_linux.o i2c_linux.o socket_linux.o
UART_RESP_OBJS = uart_resp.o
UART_SEND_OBJS = uart_send.o
SOCKET_RESP_OBJS = socket_resp.o
SOCKET_SEND_OBJS = socket_send.o
SPI_FRAM_OBJS = spi_fram.o

all: libcommbus.so uart_resp uart_send spi_fram socket_send socket_resp

uart_send: $(addprefix test/, ${UART_SEND_OBJS})
	$(CC) -o $@ ${UART_SEND_OBJS} ${LIBS} ${LDFLAGS}
uart_resp: $(addprefix test/, ${UART_RESP_OBJS})
	$(CC) -o $@ ${UART_RESP_OBJS} ${LIBS} ${LDFLAGS}
socket_send: $(addprefix test/, ${SOCKET_SEND_OBJS})
	$(CC) -o $@ ${SOCKET_SEND_OBJS} ${LIBS} ${LDFLAGS}
socket_resp: $(addprefix test/, ${SOCKET_RESP_OBJS})
	$(CC) -o $@ ${SOCKET_RESP_OBJS} ${LIBS} ${LDFLAGS}
spi_fram: $(addprefix test/, ${SPI_FRAM_OBJS})
	$(CC) -o $@ ${SPI_FRAM_OBJS} ${LIBS} ${LDFLAGS}
libcommbus.so: $(addprefix lib/, ${LIB_OBJS})
	$(CC) -shared -fPIC -o $@ ${LIB_OBJS}
libcommbus.a: $(addprefix lib/, ${LIB_OBJS})
	$(AR) rcs $@ ${LIB_OBJS}
	
%.o: %.c
	$(CC) -c -fPIC $< ${CFLAGS}
%.o: %.cpp
	$(CPP) -c -fPIC $< ${CPPFLAGS}
clean:
	rm -f libcommbus.so uart_resp uart_send spi_fram socket_send socket_resp ${UART_SEND_OBJS} ${UART_RESP_OBJS} ${SOCKET_SEND_OBJS} ${SOCKET_RESP_OBJS} ${SPI_FRAM_OBJS} ${LIB_OBJS}

