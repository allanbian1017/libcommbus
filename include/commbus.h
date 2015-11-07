#ifndef COMMBUS_H
#define COMMBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG

#if defined(_MSC_VER)

#ifdef DEBUG
#define debug_print(fmt, ...)     do { printf(fmt, __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...)     do { } while (0)
#endif

#elif defined(__GNUC__) || defined(__GNUG__)

#ifdef DEBUG
#define debug_print(fmt...)     do { printf(fmt); } while (0)
#else
#define debug_print(fmt...)     do { } while (0)
#endif

#endif

enum {
     LIBCOMMBUS_SUCCESS = 0,
     LIBCOMMBUS_ERROR_NOT_SUPPORT,
     LIBCOMMBUS_ERROR_NO_DEVICE,
     LIBCOMMBUS_ERROR_ACCESS,
     LIBCOMMBUS_ERROR_MALLOC
     
     
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
