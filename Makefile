CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
CFLAGS = -Wall -Iinclude/
CFLAGS += -I/usr/local/include/libusb-1.0/
CPPFLAGS = -Wall -Iinclude/
LIBS = -lcommbus
LDFLAGS = 
LIB_OBJS = uart_linux.o
UART_RESP_OBJS = uart_resp.o
UART_SEND_OBJS = uart_send.o

all: libcommbus.a libcommbus.so uart_resp uart_send

uart_send: $(addprefix test/, ${UART_SEND_OBJS})
	$(CC) -o $@ ${UART_SEND_OBJS} ${LIBS} ${LDFLAGS}
uart_resp: $(addprefix test/, ${UART_RESP_OBJS})
	$(CC) -o $@ ${UART_RESP_OBJS} ${LIBS} ${LDFLAGS}
libcommbus.so: $(addprefix lib/, ${LIB_OBJS})
	$(CC) -shared -fPIC -o $@ ${LIB_OBJS}
libcommbus.a: $(addprefix lib/, ${LIB_OBJS})
	$(AR) rcs $@ ${LIB_OBJS}
	
%.o: %.c
	$(CC) -c $< ${CFLAGS}
%.o: %.cpp
	$(CPP) -c $< ${CPPFLAGS}
clean:
	rm -f libcommbus.a libcommbus.so uart_resp uart_send ${UART_SEND_OBJS} ${UART_RESP_OBJS} ${LIB_OBJS}

