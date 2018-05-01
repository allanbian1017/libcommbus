#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <string.h>
#include "commbus.h"
#include "gpio.h"

static int gpio_set_dir(int pin, int dir)
{
    char buf[64];
    int fd = 0;
    int ret;

    sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    switch (dir) {
    case DIR_IN:
        ret = write(fd, "in", 2);
        if (ret < 0) {
            perror("write");
            goto fail;
        }
        break;

    case DIR_OUT:
        ret = write(fd, "out", 3);
        if (ret < 0) {
            perror("write");
            goto fail;
        }
        break;

    default:
        break;
    }

    close(fd);
    return LIBCOMMBUS_SUCCESS;

fail:
    close(fd);
    return -LIBCOMMBUS_ERROR_ACCESS;
}

int gpio_open(int pin, int dir, int val)
{
    struct stat s;
    int err;
    int ret;
    char buf[128];
    int fd = 0;
    int fd2 = 0;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    fd2 = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd2 < 0) {
        perror("open");
        goto exit;
    }

    sprintf(buf, "/sys/class/gpio/gpio%d", pin);
    err = stat(buf, &s);
    if(err != -1) {
	    sprintf(buf, "%d", pin);
	    ret = write(fd2, buf, strlen(buf));
        if (ret < 0) {
           perror("write");
           goto fail;
        }
    }

    sprintf(buf, "%d", pin);
    ret = write(fd, buf, strlen(buf));
    if (ret < 0) {
        perror("write");
        goto fail;
    }

    ret = gpio_set_dir(pin, dir);
    if (ret < 0) {
        perror("gpio_set_dir");
        goto fail;
    }

    if (dir == DIR_OUT) {
        ret = gpio_set(pin, val);
        if ( ret < 0 ) {
            perror("gpio_set");
            goto fail;
        }
    }
	
    close(fd);
    close(fd2);
    return LIBCOMMBUS_SUCCESS;

fail:
    close(fd2);
exit:
    close(fd);
    return -LIBCOMMBUS_ERROR_ACCESS;
}

int gpio_set(int pin, int val)
{
    char buf[64];
    int fd = 0;
    int ret;

    sprintf(buf, "/sys/class/gpio/gpio%d/value", pin);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    switch (val) {
    case VAL_LOW:
        ret = write(fd, "0", 1);
        if (ret < 0) {
            perror("write");
            goto fail;
        }
        break;

    case VAL_HIGH:
        ret = write(fd, "1", 1);
        if (ret < 0) {
            perror("write");
            goto fail;
        }
        break;

    default:
        break;
    }

    close(fd);
    return LIBCOMMBUS_SUCCESS;

fail:
    close(fd);
    return -LIBCOMMBUS_ERROR_ACCESS;
}

int gpio_get(int pin, int *val)
{
    char buf[64];
    int fd = 0;
    int ret;
    char ret_value;

    sprintf(buf, "/sys/class/gpio/gpio%d/value", pin);

    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    ret = read(fd, &ret_value, 1);
    if (ret < 0) {
        perror("read");
        goto fail;
    }
    
    if(ret_value == '0')
        *val = VAL_LOW;
    else if(ret_value == '1')
        *val = VAL_HIGH;
    else
        *val = VAL_UNKNOWN;

    close(fd);
    return LIBCOMMBUS_SUCCESS;

fail:
    close(fd);
    return -LIBCOMMBUS_ERROR_ACCESS;
}

int gpio_close(int pin)
{
    int ret;
    char buf[64];
    int fd = 0;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    sprintf(buf, "%d", pin);
    ret = write(fd, buf, strlen(buf));
    if (ret < 0) {
        perror("write");
        goto fail;
    }

    close(fd);
    return LIBCOMMBUS_SUCCESS;

fail:
    close(fd);
    return -LIBCOMMBUS_ERROR_ACCESS;
}
