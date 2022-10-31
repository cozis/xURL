#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef XURL_ZEROTERMINATE
#define XURL_ZEROTERMINATE 0
#endif

#if XURL_ZEROTERMINATE
#define XURL_INPUT_CONSTNESS
#else
#define XURL_INPUT_CONSTNESS const
#endif

typedef enum {
    XURL_HOSTMODE_NAME,
    XURL_HOSTMODE_IPV4,
    XURL_HOSTMODE_IPV6,
} xurl_hostmode;

typedef struct {
    xurl_hostmode mode;
    union {
        uint32_t ipv4;
        uint16_t ipv6[8];
        struct {
            XURL_INPUT_CONSTNESS char *name;
            size_t name_len;
        };
    };
    bool  no_port;
    uint16_t port;
} xurl_host;

typedef struct {
    XURL_INPUT_CONSTNESS char *username;
    XURL_INPUT_CONSTNESS char *password;
    size_t  username_len;
    size_t  password_len;
} xurl_userinfo;

typedef struct {
    xurl_host host;
    xurl_userinfo userinfo;
    XURL_INPUT_CONSTNESS char *path;
    XURL_INPUT_CONSTNESS char *query;
    XURL_INPUT_CONSTNESS char *schema;
    XURL_INPUT_CONSTNESS char *fragment;
    size_t  path_len;
    size_t  query_len;
    size_t  schema_len;
    size_t  fragment_len;
#if XURL_ZEROTERMINATE
    char buffer[512];
#endif
} xurl_t;

bool xurl_parse2(XURL_INPUT_CONSTNESS char *src, size_t len, size_t *i, xurl_t *url);
bool xurl_parse(XURL_INPUT_CONSTNESS char *src, size_t len, xurl_t *url);
bool xurl_parse_ipv6(const char *src, size_t len, uint16_t out[8]);
bool xurl_parse_ipv4(const char *src, size_t len, uint32_t *out);