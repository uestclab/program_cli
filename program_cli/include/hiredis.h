#ifndef __HIREDIS_H
#define __HIREDIS_H
#include <stdarg.h> /* for va_list */
#include <sys/time.h> /* for struct timeval */
#include <stdint.h> /* uintXX_t, etc */
#include "sds.h" /* for sds */

#define HIREDIS_MAJOR 0
#define HIREDIS_MINOR 13
#define HIREDIS_PATCH 3
#define HIREDIS_SONAME 0.13

/* Connection type can be blocking or non-blocking and is set in the
 * least significant bit of the flags field in redisContext. */
#define REDIS_BLOCK 0x1

/* Connection may be disconnected before being free'd. The second bit
 * in the flags field is set when the context is connected. */
#define REDIS_CONNECTED 0x2

/* The async API might try to disconnect cleanly and flush the output
 * buffer and read all subsequent replies before disconnecting.
 * This flag means no new commands can come in and the connection
 * should be terminated once all replies have been read. */
#define REDIS_DISCONNECTING 0x4

/* Flag specific to the async API which means that the context should be clean
 * up as soon as possible. */
#define REDIS_FREEING 0x8

/* Flag that is set when an async callback is executed. */
#define REDIS_IN_CALLBACK 0x10

/* Flag that is set when the async context has one or more subscriptions. */
#define REDIS_SUBSCRIBED 0x20

/* Flag that is set when monitor mode is active */
#define REDIS_MONITORING 0x40

/* Flag that is set when we should set SO_REUSEADDR before calling bind() */
#define REDIS_REUSEADDR 0x80

#define REDIS_KEEPALIVE_INTERVAL 15 /* seconds */

/* number of times we retry to connect in the case of EADDRNOTAVAIL and
 * SO_REUSEADDR is being used. */
#define REDIS_CONNECT_RETRIES  10

/* strerror_r has two completely different prototypes and behaviors
 * depending on system issues, so we need to operate on the error buffer
 * differently depending on which strerror_r we're using. */
#ifndef _GNU_SOURCE
/* "regular" POSIX strerror_r that does the right thing. */
#define __redis_strerror_r(errno, buf, len)                                    \
    do {                                                                       \
        strerror_r((errno), (buf), (len));                                     \
    } while (0)
#else
/* "bad" GNU strerror_r we need to clean up after. */
#define __redis_strerror_r(errno, buf, len)                                    \
    do {                                                                       \
        char *err_str = strerror_r((errno), (buf), (len));                     \
        /* If return value _isn't_ the start of the buffer we passed in,       \
         * then GNU strerror_r returned an internal static buffer and we       \
         * need to copy the result into our private buffer. */                 \
        if (err_str != (buf)) {                                                \
            strncpy((buf), err_str, ((len) - 1));                              \
            buf[(len)-1] = '\0';                                               \
        }                                                                      \
    } while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define REDIS_ERR -1
#define REDIS_OK 0

enum redisConnectionType {
    REDIS_CONN_TCP,
    REDIS_CONN_UNIX
};

/* Context for a connection to Redis */
typedef struct redisContext {
    int err; /* Error flags, 0 when there is no error */
    char errstr[128]; /* String representation of error when applicable */
    int fd;
    int flags;
    char *obuf; /* Write buffer */
    // redisReader *reader; /* Protocol reader */

    enum redisConnectionType connection_type;
    struct timeval *timeout;

    struct {
        char *host;
        char *source_addr;
        int port;
    } tcp;

    struct {
        char *path;
    } unix_sock;

} redisContext;


void redisFree(redisContext *c);
redisContext *redisConnect(const char *ip, int port);


#ifdef __cplusplus
}
#endif

#endif