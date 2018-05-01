#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

enum{
        DIR_IN = 0,
        DIR_OUT,
};

enum{
        VAL_LOW = 0,
        VAL_HIGH,
        VAL_UNKNOWN
};

extern int gpio_open(int pin, int dir, int val);
extern int gpio_set(int pin, int val);
extern int gpio_get(int pin, int *val);
extern int gpio_close(int pin);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
