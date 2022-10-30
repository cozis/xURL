#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "xurl.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct {
    const char *schema;
    const char *username;
    const char *password;
    const char *host_name;
    const char *host_ipv4;
    const char *host_ipv6;
    uint16_t port;
    const char *abs_path;
    const char *rel_path;
    const char *query;
    const char *fragment;
} URLConfig;

static bool generate_uri(size_t i, const URLConfig *config, 
                         char *dst, size_t *len, size_t max,
                         xurl_t *out)
{
    const char *host_name = config->host_name;
    const char *host_ipv4 = config->host_ipv4;
    const char *host_ipv6 = config->host_ipv6;
    const char *rel_path = config->rel_path;
    const char *abs_path = config->abs_path;
    const char *fragment = config->fragment;
    const char *query = config->query;
    uint16_t port = config->port;
    const char *username = config->username;
    const char *password = config->password;
    const char *schema = config->schema;

    enum {
        HAVE_REL_PATH  = 1 << 0,
        HAVE_ABS_PATH  = 1 << 1,
        HAVE_QUERY     = 1 << 2,
        HAVE_FRAGMENT  = 1 << 3,
        HAVE_HOST_NAME = 1 << 4,
        HAVE_HOST_IPV4 = 1 << 5,
        HAVE_HOST_IPV6 = 1 << 6,
        HAVE_PORT      = 1 << 7,
        HAVE_USERNAME  = 1 << 8,
        HAVE_PASSWORD  = 1 << 9,
        HAVE_SCHEMA    = 1 << 10,
    };

    int k, flags;
    switch (i) {
        
        case __COUNTER__: k = snprintf(dst, max, "%s",                           rel_path                 );       flags = HAVE_REL_PATH;                                               break;
        case __COUNTER__: k = snprintf(dst, max, "%s#%s",                        rel_path,        fragment);       flags = HAVE_REL_PATH | HAVE_FRAGMENT;                               break;
        case __COUNTER__: k = snprintf(dst, max, "%s?%s",                        rel_path, query          );       flags = HAVE_REL_PATH | HAVE_QUERY;                                  break;
        case __COUNTER__: k = snprintf(dst, max, "%s?%s#%s",                     rel_path, query, fragment);       flags = HAVE_REL_PATH | HAVE_QUERY | HAVE_FRAGMENT;                  break;
        case __COUNTER__: k = snprintf(dst, max, "%s",                           abs_path                 );       flags = HAVE_ABS_PATH;                                               break;
        case __COUNTER__: k = snprintf(dst, max, "%s#%s",                        abs_path,        fragment);       flags = HAVE_ABS_PATH | HAVE_FRAGMENT;                               break;
        case __COUNTER__: k = snprintf(dst, max, "%s?%s",                        abs_path, query          );       flags = HAVE_ABS_PATH | HAVE_QUERY;                                  break;
        case __COUNTER__: k = snprintf(dst, max, "%s?%s#%s",                     abs_path, query, fragment);       flags = HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT;                  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s",              host_name);                                  flags = HAVE_HOST_NAME;                                              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s#%s",           host_name, fragment);                        flags = HAVE_HOST_NAME | HAVE_FRAGMENT;                              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s?%s",           host_name, query);                           flags = HAVE_HOST_NAME | HAVE_QUERY;                                 break;
        case __COUNTER__: k = snprintf(dst, max, "//%s?%s#%s",        host_name, query, fragment);                 flags = HAVE_HOST_NAME | HAVE_QUERY | HAVE_FRAGMENT;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s",            host_name, abs_path);                        flags = HAVE_HOST_NAME | HAVE_ABS_PATH;                              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s#%s",         host_name, abs_path, fragment);              flags = HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_FRAGMENT;              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s?%s",         host_name, abs_path, query);                 flags = HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s?%s#%s",      host_name, abs_path, query, fragment);       flags = HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d",           host_name, port);                            flags = HAVE_HOST_NAME | HAVE_PORT;                                  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d#%s",        host_name, port, fragment);                  flags = HAVE_HOST_NAME | HAVE_PORT | HAVE_FRAGMENT;                  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d?%s",        host_name, port, query);                     flags = HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY;                     break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d?%s#%s",     host_name, port, query, fragment);           flags = HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT;     break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s",         host_name, port, abs_path);                  flags = HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH;                  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s#%s",      host_name, port, abs_path, fragment);        flags = HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT;  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s?%s",      host_name, port, abs_path, query);           flags = HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY;     break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s?%s#%s",   host_name, port, abs_path, query, fragment); flags = HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s",              host_ipv4);                                  flags = HAVE_HOST_IPV4;                                              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s#%s",           host_ipv4, fragment);                        flags = HAVE_HOST_IPV4 | HAVE_FRAGMENT;                              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s?%s",           host_ipv4, query);                           flags = HAVE_HOST_IPV4 | HAVE_QUERY;                                 break;
        case __COUNTER__: k = snprintf(dst, max, "//%s?%s#%s",        host_ipv4, query, fragment);                 flags = HAVE_HOST_IPV4 | HAVE_QUERY | HAVE_FRAGMENT;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s",            host_ipv4, abs_path);                        flags = HAVE_HOST_IPV4 | HAVE_ABS_PATH;                              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s#%s",         host_ipv4, abs_path, fragment);              flags = HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_FRAGMENT;              break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s?%s",         host_ipv4, abs_path, query);                 flags = HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//%s%s?%s#%s",      host_ipv4, abs_path, query, fragment);       flags = HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d",           host_ipv4, port);                            flags = HAVE_HOST_IPV4 | HAVE_PORT;                                  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d#%s",        host_ipv4, port, fragment);                  flags = HAVE_HOST_IPV4 | HAVE_PORT | HAVE_FRAGMENT;                  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d?%s",        host_ipv4, port, query);                     flags = HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY;                     break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d?%s#%s",     host_ipv4, port, query, fragment);           flags = HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT;     break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s",         host_ipv4, port, abs_path);                  flags = HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH;                  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s#%s",      host_ipv4, port, abs_path, fragment);        flags = HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT;  break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s?%s",      host_ipv4, port, abs_path, query);           flags = HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY;     break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%d%s?%s#%s",   host_ipv4, port, abs_path, query, fragment); flags = HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]",            host_ipv6);                                  flags = HAVE_HOST_IPV6;                                              break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]#%s",         host_ipv6, fragment);                        flags = HAVE_HOST_IPV6 | HAVE_FRAGMENT;                              break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]?%s",         host_ipv6, query);                           flags = HAVE_HOST_IPV6 | HAVE_QUERY;                                 break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]?%s#%s",      host_ipv6, query, fragment);                 flags = HAVE_HOST_IPV6 | HAVE_QUERY | HAVE_FRAGMENT;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]%s",          host_ipv6, abs_path);                        flags = HAVE_HOST_IPV6 | HAVE_ABS_PATH;                              break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]%s#%s",       host_ipv6, abs_path, fragment);              flags = HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_FRAGMENT;              break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]%s?%s",       host_ipv6, abs_path, query);                 flags = HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]%s?%s#%s",    host_ipv6, abs_path, query, fragment);       flags = HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d",         host_ipv6, port);                            flags = HAVE_HOST_IPV6 | HAVE_PORT;                                  break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d#%s",      host_ipv6, port, fragment);                  flags = HAVE_HOST_IPV6 | HAVE_PORT | HAVE_FRAGMENT;                  break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d?%s",      host_ipv6, port, query);                     flags = HAVE_HOST_IPV6 | HAVE_QUERY;                                 break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d?%s#%s",   host_ipv6, port, query, fragment);           flags = HAVE_HOST_IPV6 | HAVE_QUERY | HAVE_FRAGMENT;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d%s",       host_ipv6, port, abs_path);                  flags = HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH;                  break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d%s#%s",    host_ipv6, port, abs_path, fragment);        flags = HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT;  break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d%s?%s",    host_ipv6, port, abs_path, query);           flags = HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY;     break;
        case __COUNTER__: k = snprintf(dst, max, "//[%s]:%d%s?%s#%s", host_ipv6, port, abs_path, query, fragment); flags = HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s",       username, host_name);                            flags = HAVE_USERNAME | HAVE_HOST_NAME; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s#%s",    username, host_name, fragment);                  flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s?%s",    username, host_name, query);                     flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s?%s#%s", username, host_name, query, fragment);           flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s",       username, host_name, abs_path);                flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s#%s",    username, host_name, abs_path, fragment);      flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s?%s",    username, host_name, abs_path, query);         flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s?%s#%s", username, host_name, abs_path, query, fragment); flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d",       username, host_name, port);                   flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d#%s",    username, host_name, port, fragment);         flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d?%s",    username, host_name, port, query);            flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d?%s#%s", username, host_name, port, query, fragment);  flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s",       username, host_name, port, abs_path);                    flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s#%s",    username, host_name, port, abs_path, fragment);          flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s?%s",    username, host_name, port, abs_path, query);             flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s?%s#%s", username, host_name, port, abs_path, query, fragment);   flags = HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s",       username, host_ipv4);                                         flags = HAVE_USERNAME | HAVE_HOST_IPV4; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s#%s",    username, host_ipv4, fragment);                               flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s?%s",    username, host_ipv4, query);                                  flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s?%s#%s", username, host_ipv4, query, fragment);                        flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s",       username, host_ipv4, abs_path);                             flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s#%s",    username, host_ipv4, abs_path, fragment);                   flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s?%s",    username, host_ipv4, abs_path, query);                      flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s%s?%s#%s", username, host_ipv4, abs_path, query, fragment);            flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d",       username, host_ipv4, port);                                flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d#%s",    username, host_ipv4, port, fragment);                      flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d?%s",    username, host_ipv4, port, query);                         flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d?%s#%s", username, host_ipv4, port, query, fragment);               flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s",       username, host_ipv4, port, abs_path);                    flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s#%s",    username, host_ipv4, port, abs_path, fragment);          flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s?%s",    username, host_ipv4, port, abs_path, query);             flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@%s:%d%s?%s#%s", username, host_ipv4, port, abs_path, query, fragment);   flags = HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]",       username, host_ipv6);                                       flags = HAVE_USERNAME | HAVE_HOST_IPV6;                 break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]#%s",    username, host_ipv6, fragment);                             flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]?%s",    username, host_ipv6, query);                                flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_QUERY;    break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]?%s#%s", username, host_ipv6, query, fragment);                      flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]%s",       username, host_ipv6, abs_path);                           flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]%s#%s",    username, host_ipv6, abs_path, fragment);                 flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]%s?%s",    username, host_ipv6, abs_path, query);                    flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]%s?%s#%s", username, host_ipv6, abs_path, query, fragment);          flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d",       username, host_ipv6, port);                              flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d#%s",    username, host_ipv6, port, fragment);                    flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d?%s",    username, host_ipv6, port, query);                       flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d?%s#%s", username, host_ipv6, port, query, fragment);             flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d%s",       username, host_ipv6, port, abs_path);                  flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d%s#%s",    username, host_ipv6, port, abs_path, fragment);        flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d%s?%s",    username, host_ipv6, port, abs_path, query);           flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s@[%s]:%d%s?%s#%s", username, host_ipv6, port, abs_path, query, fragment); flags = HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s",       username, password, host_name);                            flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s#%s",    username, password, host_name, fragment);                  flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s?%s",    username, password, host_name, query);                     flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s?%s#%s", username, password, host_name, query, fragment);           flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s",       username, password, host_name, abs_path);                            flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s#%s",    username, password, host_name, abs_path, fragment);                  flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s?%s",    username, password, host_name, abs_path, query);                     flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s?%s#%s", username, password, host_name, abs_path, query, fragment);           flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d",       username, password, host_name, port);                               flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d#%s",    username, password, host_name, port, fragment);                     flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d?%s",    username, password, host_name, port, query);                        flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d?%s#%s", username, password, host_name, port, query, fragment);              flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s",       username, password, host_name, port, abs_path);                   flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s#%s",    username, password, host_name, port, abs_path, fragment);         flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s?%s",    username, password, host_name, port, abs_path, query);            flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s?%s#%s", username, password, host_name, port, abs_path, query, fragment);  flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s",       username, password, host_ipv4);                                        flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s#%s",    username, password, host_ipv4, fragment);                              flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s?%s",    username, password, host_ipv4, query);                                 flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s?%s#%s", username, password, host_ipv4, query, fragment);                       flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s",       username, password, host_ipv4, abs_path);                            flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s#%s",    username, password, host_ipv4, abs_path, fragment);                  flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s?%s",    username, password, host_ipv4, abs_path, query);                     flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s%s?%s#%s", username, password, host_ipv4, abs_path, query, fragment);           flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d",       username, password, host_ipv4, port);                               flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d#%s",    username, password, host_ipv4, port, fragment);                     flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d?%s",    username, password, host_ipv4, port, query);                        flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d?%s#%s", username, password, host_ipv4, port, query, fragment);              flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s",       username, password, host_ipv4, port, abs_path);                   flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s#%s",    username, password, host_ipv4, port, abs_path, fragment);         flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s?%s",    username, password, host_ipv4, port, abs_path, query);            flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@%s:%d%s?%s#%s", username, password, host_ipv4, port, abs_path, query, fragment);  flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]",       username, password, host_ipv6);                                      flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]#%s",    username, password, host_ipv6, fragment);                            flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]?%s",    username, password, host_ipv6, query);                               flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]?%s#%s", username, password, host_ipv6, query, fragment);                     flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]%s",       username, password, host_ipv6, abs_path);                          flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]%s#%s",    username, password, host_ipv6, abs_path, fragment);                flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]%s?%s",    username, password, host_ipv6, abs_path, query);                   flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]%s?%s#%s", username, password, host_ipv6, abs_path, query, fragment);         flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d",       username, password, host_ipv6, port);                             flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d#%s",    username, password, host_ipv6, port, fragment);                   flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d?%s",    username, password, host_ipv6, port, query);                      flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d?%s#%s", username, password, host_ipv6, port, query, fragment);            flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;        
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d%s",       username, password, host_ipv6, port, abs_path);                 flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d%s#%s",    username, password, host_ipv6, port, abs_path, fragment);       flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d%s?%s",    username, password, host_ipv6, port, abs_path, query);          flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "//%s:%s@[%s]:%d%s?%s#%s", username, password, host_ipv6, port, abs_path, query, fragment);flags = HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s",       schema, rel_path);                                          flags = HAVE_SCHEMA | HAVE_REL_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s#%s",    schema, rel_path, fragment);                                flags = HAVE_SCHEMA | HAVE_REL_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s?%s",    schema, rel_path, query);                                   flags = HAVE_SCHEMA | HAVE_REL_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s?%s#%s", schema, rel_path, query, fragment);                         flags = HAVE_SCHEMA | HAVE_REL_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s",       schema, abs_path);                                          flags = HAVE_SCHEMA | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s#%s",    schema, abs_path, fragment);                                flags = HAVE_SCHEMA | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s?%s",    schema, abs_path, query);                                   flags = HAVE_SCHEMA | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s:%s?%s#%s", schema, abs_path, query, fragment);                         flags = HAVE_SCHEMA | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s",       schema, host_name);                                       flags = HAVE_SCHEMA | HAVE_HOST_NAME; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s#%s",    schema, host_name, fragment);                             flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s?%s",    schema, host_name, query);                                flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s?%s#%s", schema, host_name, query, fragment);                      flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s",       schema, host_name, abs_path);                           flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s#%s",    schema, host_name, abs_path, fragment);                 flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s?%s",    schema, host_name, abs_path, query);                    flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s?%s#%s", schema, host_name, abs_path, query, fragment);          flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d",       schema, host_name, port);                              flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d#%s",    schema, host_name, port, fragment);                    flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d?%s",    schema, host_name, port, query);                       flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d?%s#%s", schema, host_name, port, query, fragment);             flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s",       schema, host_name, port, abs_path);                  flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s#%s",    schema, host_name, port, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s?%s",    schema, host_name, port, abs_path, query);           flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s?%s#%s", schema, host_name, port, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s",       schema, host_ipv4);                                       flags = HAVE_SCHEMA | HAVE_HOST_IPV4; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s#%s",    schema, host_ipv4, fragment);                             flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s?%s",    schema, host_ipv4, query);                                flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s?%s#%s", schema, host_ipv4, query, fragment);                      flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s",       schema, host_ipv4, abs_path);                           flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s#%s",    schema, host_ipv4, abs_path, fragment);                 flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s?%s",    schema, host_ipv4, abs_path, query);                    flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s%s?%s#%s", schema, host_ipv4, abs_path, query, fragment);          flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;        
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d",       schema, host_ipv4, port);                              flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d#%s",    schema, host_ipv4, port, fragment);                    flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d?%s",    schema, host_ipv4, port, query);                       flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d?%s#%s", schema, host_ipv4, port, query, fragment);             flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s",       schema, host_ipv4, port, abs_path);                  flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s#%s",    schema, host_ipv4, port, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s?%s",    schema, host_ipv4, port, abs_path, query);           flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%d%s?%s#%s", schema, host_ipv4, port, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]",       schema, host_ipv6);                                     flags = HAVE_SCHEMA | HAVE_HOST_IPV6; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]#%s",    schema, host_ipv6, fragment);                           flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]?%s",    schema, host_ipv6, query);                              flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]?%s#%s", schema, host_ipv6, query, fragment);                    flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]%s",       schema, host_ipv6, abs_path);                         flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]%s#%s",    schema, host_ipv6, abs_path, fragment);               flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]%s?%s",    schema, host_ipv6, abs_path, query);                  flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]%s?%s#%s", schema, host_ipv6, abs_path, query, fragment);        flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d",       schema, host_ipv6, port);                            flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d#%s",    schema, host_ipv6, port, fragment);                  flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d?%s",    schema, host_ipv6, port, query);                     flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d?%s#%s", schema, host_ipv6, port, query, fragment);           flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d%s",       schema, host_ipv6, port, abs_path);                  flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d%s#%s",    schema, host_ipv6, port, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d%s?%s",    schema, host_ipv6, port, abs_path, query);           flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://[%s]:%d%s?%s#%s", schema, host_ipv6, port, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s",       schema, username, host_name); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s#%s",    schema, username, host_name, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s?%s",    schema, username, host_name, query); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s?%s#%s", schema, username, host_name, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s",       schema, username, host_name, abs_path); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s#%s",    schema, username, host_name, abs_path, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s?%s",    schema, username, host_name, abs_path, query); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s?%s#%s", schema, username, host_name, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d",       schema, username, host_name, port); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d#%s",    schema, username, host_name, port, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d?%s",    schema, username, host_name, port, query); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d?%s#%s", schema, username, host_name, port, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s",       schema, username, host_name, port, abs_path); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s#%s",    schema, username, host_name, port, abs_path, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s?%s",    schema, username, host_name, port, abs_path, query); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s?%s#%s", schema, username, host_name, port, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s",       schema, username, host_ipv4);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s#%s",    schema, username, host_ipv4, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s?%s",    schema, username, host_ipv4, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s?%s#%s", schema, username, host_ipv4, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s",       schema, username, host_ipv4, abs_path);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s#%s",    schema, username, host_ipv4, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s?%s",    schema, username, host_ipv4, abs_path, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s%s?%s#%s", schema, username, host_ipv4, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d",       schema, username, host_ipv4, port);                     flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d#%s",    schema, username, host_ipv4, port, fragment);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d?%s",    schema, username, host_ipv4, port, query);              flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d?%s#%s", schema, username, host_ipv4, port, query, fragment);    flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s",       schema, username, host_ipv4, port, abs_path);                     flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s#%s",    schema, username, host_ipv4, port, abs_path, fragment);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s?%s",    schema, username, host_ipv4, port, abs_path, query);              flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@%s:%d%s?%s#%s", schema, username, host_ipv4, port, abs_path, query, fragment);    flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]",       schema, username, host_ipv6);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]#%s",    schema, username, host_ipv6, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]?%s",    schema, username, host_ipv6, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]?%s#%s", schema, username, host_ipv6, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]%s",       schema, username, host_ipv6, abs_path);                    flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]%s#%s",    schema, username, host_ipv6, abs_path, fragment);          flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]%s?%s",    schema, username, host_ipv6, abs_path, query);             flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]%s?%s#%s", schema, username, host_ipv6, abs_path, query, fragment);   flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d",       schema, username, host_ipv6, port);                   flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d#%s",    schema, username, host_ipv6, port, fragment);         flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d?%s",    schema, username, host_ipv6, port, query);            flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d?%s#%s", schema, username, host_ipv6, port, query, fragment);  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d%s",       schema, username, host_ipv6, port, abs_path);                   flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d%s#%s",    schema, username, host_ipv6, port, abs_path, fragment);         flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d%s?%s",    schema, username, host_ipv6, port, abs_path, query);            flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s@[%s]:%d%s?%s#%s", schema, username, host_ipv6, port, abs_path, query, fragment);  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s",            schema, username, password, host_name);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s#%s",         schema, username, password, host_name, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s?%s",         schema, username, password, host_name, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s?%s#%s",      schema, username, password, host_name, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s",          schema, username, password, host_name, abs_path);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s#%s",       schema, username, password, host_name, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s?%s",       schema, username, password, host_name, abs_path, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s?%s#%s",    schema, username, password, host_name, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d",         schema, username, password, host_name, port);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d#%s",      schema, username, password, host_name, port, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d?%s",      schema, username, password, host_name, port, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d?%s#%s",   schema, username, password, host_name, port, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s",       schema, username, password, host_name, port, abs_path);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s#%s",    schema, username, password, host_name, port, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s?%s",    schema, username, password, host_name, port, abs_path, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s?%s#%s", schema, username, password, host_name, port, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_NAME | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s",       schema, username, password, host_ipv4);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s#%s",    schema, username, password, host_ipv4, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s?%s",    schema, username, password, host_ipv4, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s?%s#%s", schema, username, password, host_ipv4, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s",       schema, username, password, host_ipv4, abs_path);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s#%s",    schema, username, password, host_ipv4, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s?%s",    schema, username, password, host_ipv4, abs_path, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s%s?%s#%s", schema, username, password, host_ipv4, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d",       schema, username, password, host_ipv4, port);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d#%s",    schema, username, password, host_ipv4, port, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d?%s",    schema, username, password, host_ipv4, port, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d?%s#%s", schema, username, password, host_ipv4, port, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s",       schema, username, password, host_ipv4, port, abs_path);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s#%s",    schema, username, password, host_ipv4, port, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s?%s",    schema, username, password, host_ipv4, port, abs_path, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@%s:%d%s?%s#%s", schema, username, password, host_ipv4, port, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV4 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]",       schema, username, password, host_ipv6);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]#%s",    schema, username, password, host_ipv6, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]?%s",    schema, username, password, host_ipv6, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]?%s#%s", schema, username, password, host_ipv6, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]%s",       schema, username, password, host_ipv6, abs_path);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]%s#%s",    schema, username, password, host_ipv6, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]%s?%s",    schema, username, password, host_ipv6, abs_path, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]%s?%s#%s", schema, username, password, host_ipv6, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d",       schema, username, password, host_ipv6, port);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d#%s",    schema, username, password, host_ipv6, port, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d?%s",    schema, username, password, host_ipv6, port, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d?%s#%s", schema, username, password, host_ipv6, port, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_QUERY | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d%s",       schema, username, password, host_ipv6, port, abs_path);                  flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d%s#%s",    schema, username, password, host_ipv6, port, abs_path, fragment);        flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_FRAGMENT; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d%s?%s",    schema, username, password, host_ipv6, port, abs_path, query);           flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY; break;
        case __COUNTER__: k = snprintf(dst, max, "%s://%s:%s@[%s]:%d%s?%s#%s", schema, username, password, host_ipv6, port, abs_path, query, fragment); flags = HAVE_SCHEMA | HAVE_USERNAME | HAVE_PASSWORD | HAVE_HOST_IPV6 | HAVE_PORT | HAVE_ABS_PATH | HAVE_QUERY | HAVE_FRAGMENT; break;
        default: 
        return false;
    }

    if (k < 0) 
        abort();
    
    if (len) 
        *len = (size_t) k;

    if (flags & HAVE_REL_PATH) {
        out->path = rel_path;
        out->path_len = strlen(rel_path);
    } else if (flags & HAVE_ABS_PATH) {
        out->path = abs_path;
        out->path_len = strlen(abs_path);
    } else {
        out->path = NULL;
        out->path_len = 0;
    }

    if (flags & HAVE_SCHEMA) {
        out->schema = schema;
        out->schema_len = strlen(schema);
    } else {
        out->schema = NULL;
        out->schema_len = 0;
    }

    if (flags & HAVE_QUERY) {
        out->query = query;
        out->query_len = strlen(query);
    } else {
        out->query = NULL;
        out->query_len = 0;
    }

    if (flags & HAVE_FRAGMENT) {
        out->fragment = fragment;
        out->fragment_len = strlen(fragment);
    } else {
        out->fragment = NULL;
        out->fragment_len = 0;
    }

    if (flags & HAVE_HOST_NAME) {
        out->host.mode = XURL_HOSTMODE_NAME;
        out->host.name = host_name;
        out->host.name_len = strlen(host_name);
    } else if (flags & HAVE_HOST_IPV4) {
        out->host.mode = XURL_HOSTMODE_IPV4;
        out->host.ipv4 = ntohl(inet_addr(host_ipv4)); // Endianess?
    } else if (flags & HAVE_HOST_IPV6) {
        out->host.mode = XURL_HOSTMODE_IPV6;
        // out->host.ipv6 = inet_addr(host_ipv6); // Endianess?
    } else {
        out->host.mode = XURL_HOSTMODE_NAME;
        out->host.name = NULL;
        out->host.name_len = 0;
    }

    if (flags & HAVE_PORT) {
        out->host.no_port = false;
        out->host.port = port;
    } else {
        out->host.no_port = true;
        out->host.port = 0;
    }

    if (flags & HAVE_USERNAME) {
        out->userinfo.username = username;
        out->userinfo.username_len = strlen(username);
    } else {
        out->userinfo.username = NULL;
        out->userinfo.username_len = 0;
    }

    if (flags & HAVE_PASSWORD) {
        out->userinfo.password = password;
        out->userinfo.password_len = strlen(password);
    } else {
        out->userinfo.password = NULL;
        out->userinfo.password_len = 0;
    }

    return true;
}

static void check_(const char *file, size_t line, bool res)
{
    if (!res) {
        FILE *fp = stderr;
        fprintf(fp, "Check failed at %s:%ld\n", file, line);
        fflush(fp);
        abort();
    }
}
#define check(res) check_(__FILE__, __LINE__, (res))

static const char *hostmodeName(xurl_hostmode mode)
{
    switch (mode) {
        case XURL_HOSTMODE_NAME: return "XURL_HOSTMODE_NAME";
        case XURL_HOSTMODE_IPV4: return "XURL_HOSTMODE_IPV4";
        case XURL_HOSTMODE_IPV6: return "XURL_HOSTMODE_IPV6";
    }
    return "???";
}

static bool compare_results(xurl_t *out, xurl_t *exp, FILE *fp)
{
    bool failed = false;

    if (exp->host.mode == XURL_HOSTMODE_NAME && exp->host.name == NULL) {
        if (out->host.mode != XURL_HOSTMODE_NAME || out->host.name != NULL) {
            fprintf(fp, "  Expected no host string, but got one instead\n");
            failed = true;
        }
    } else {

        if (out->host.mode != exp->host.mode) {
            fprintf(fp, "  Got host mode %s, but %s was expected\n", 
                    hostmodeName(out->host.mode), hostmodeName(exp->host.mode));
            failed = true;
        } else {
            switch (out->host.mode) {
                case XURL_HOSTMODE_NAME:
                if (out->host.name == NULL) {
                    fprintf(fp, "  Missing host name string\n");
                    failed = true;
                } else {
                    if (out->host.name_len != exp->host.name_len) {
                        fprintf(fp, "  Host name length doesn't match\n");
                        failed = true;
                    } else if (out->host.name == NULL && strncmp(out->host.name, exp->host.name, out->host.name_len)) {
                        fprintf(fp, "  Host name string doesn't match\n");
                        failed = true;
                    }
                }
                break;
                case XURL_HOSTMODE_IPV4:
                if (out->host.ipv4 != exp->host.ipv4) {
                    fprintf(fp, "  IPv4 address doesn't match\n");
                    failed = true;
                }
                break;
                case XURL_HOSTMODE_IPV6:
                if (memcmp(out->host.ipv6, exp->host.ipv6, 16)) {
                    fprintf(fp, "  IPv6 address doesn't match\n");
                    failed = true;
                }
                break;
            }
        }
    }

    if (exp->host.no_port) {
        if (out->host.no_port == false) {
            fprintf(fp, "  Expected no port, but got one\n");
            failed = true;
        }
    } else {
        if (out->host.no_port) {
            fprintf(fp, "  Expected a port, but got none\n");
            failed = true;
        } else if (exp->host.port != out->host.port) {
            fprintf(fp, "  Port mismatch (got %d but expected %d)\n",
                    out->host.port, exp->host.port);
            failed = true;
        }
    }

    if (exp->userinfo.username == NULL) {
        if (out->userinfo.username != NULL) {
            fprintf(fp, "  Empty username expected, but got one instead\n");
            failed = true;
        }
    } else {
        if (out->userinfo.username == NULL) {
            fprintf(fp, "  Username expected, but got none instead\n");
            failed = true;
        } else if (out->userinfo.username_len != exp->userinfo.username_len) {
            fprintf(fp, "  Username length mismatch %ld != %ld (expected \"%.*s\", got \"%.*s\")\n", 
                    exp->userinfo.username_len, out->userinfo.username_len,
                    (int) exp->userinfo.username_len, exp->userinfo.username, 
                    (int) out->userinfo.username_len, out->userinfo.username);
            failed = true;
        } else if (strncmp(out->userinfo.username, exp->userinfo.username, exp->userinfo.username_len)) {
            fprintf(fp, "  Username string mismatch (expected \"%.*s\", got \"%.*s\")\n", 
                    (int) exp->userinfo.username_len, exp->userinfo.username, 
                    (int) out->userinfo.username_len, out->userinfo.username);
            failed = true;
        }
    }

    if (exp->userinfo.password == NULL) {
        if (out->userinfo.password != NULL) {
            fprintf(fp, "  Empty password expected, but got one instead\n");
            failed = true;
        }
    } else {
        if (out->userinfo.password == NULL) {
            fprintf(fp, "  Password expected, but got none instead\n");
            failed = true;
        } else if (out->userinfo.password_len != exp->userinfo.password_len) {
            fprintf(fp, "  Password length mismatch %ld != %ld (expected \"%.*s\", got \"%.*s\")\n", 
                    exp->userinfo.username_len, out->userinfo.username_len,
                    (int) exp->userinfo.password_len, exp->userinfo.password, 
                    (int) out->userinfo.password_len, out->userinfo.password);
            failed = true;
        } else if (strncmp(out->userinfo.password, exp->userinfo.password, exp->userinfo.password_len)) {
            fprintf(fp, "  Password string mismatch (expected \"%.*s\", got \"%.*s\")\n", 
                    (int) exp->userinfo.password_len, exp->userinfo.password, 
                    (int) out->userinfo.password_len, out->userinfo.password);
            failed = true;
        }
    }

    if (exp->path == NULL) {
        if (out->path != NULL) {
            fprintf(fp, "  Empty path expected, got one instead\n");
            failed = true;
        }
    } else {
        if (out->path == NULL) {
            fprintf(fp, "  Path expected, got none instead\n");
            failed = true;
        } else if (out->path_len != exp->path_len) {
            fprintf(fp, "  Path length mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->path_len, out->path, 
                    (int) exp->path_len, exp->path);
            failed = true;
        } else if (strncmp(out->path, exp->path, exp->path_len)) {
            fprintf(fp, "  Path string mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->path_len, out->path, 
                    (int) exp->path_len, exp->path);
            failed = true;
        }
    }

    if (exp->query == NULL) {
        if (out->query != NULL) {
            fprintf(fp, "  Empty query expected, got one instead\n");
            failed = true;
        }
    } else {
        if (out->query == NULL) {
            fprintf(fp, "  Query expected, got none instead\n");
            failed = true;
        } else if (out->query_len != exp->query_len) {
            fprintf(fp, "  Query length mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->query_len, out->query, 
                    (int) exp->query_len, exp->query);
            failed = true;
        } else if (strncmp(out->query, exp->query, exp->query_len)) {
            fprintf(fp, "  Query string mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->query_len, out->query, 
                    (int) exp->query_len, exp->query);
            failed = true;
        }
    }

    if (exp->fragment == NULL) {
        if (out->fragment != NULL) {
            fprintf(fp, "  Empty fragment expected, got one instead\n");
            failed = true;
        }
    } else {
        if (out->fragment == NULL) {
            fprintf(fp, "  Fragment expected, got none instead\n");
            failed = true;
        } else if (out->fragment_len != exp->fragment_len) {
            fprintf(fp, "  Fragment length mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->fragment_len, out->fragment, (int) exp->fragment_len, exp->fragment);
            failed = true;
        } else if (strncmp(out->fragment, exp->fragment, exp->fragment_len)) {
            fprintf(fp, "  Fragment string mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->fragment_len, out->fragment, (int) exp->fragment_len, exp->fragment);
            failed = true;
        }
    }

    if (exp->schema == NULL) {
        if (out->schema != NULL) {
            fprintf(fp, "  Empty schema expected, got one instead\n");
            failed = true;
        }
    } else {
        if (out->schema == NULL) {
            fprintf(fp, "  Schema expected, got none instead\n");
            failed = true;
        } else if (out->schema_len != exp->schema_len) {
            fprintf(fp, "  Schema length mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->schema_len, out->schema, (int) exp->schema_len, exp->schema);
            failed = true;
        } else if (strncmp(out->schema, exp->schema, exp->schema_len)) {
            fprintf(fp, "  Schema string mismatch (got \"%.*s\", expected \"%.*s\")\n",
                    (int) out->schema_len, out->schema, (int) exp->schema_len, exp->schema);
            failed = true;
        }
    }

    return !failed;
}

int main(void)
{
    printf("%d testcases\n", __COUNTER__);
    static const URLConfig config = {
        .schema = "http",
        .username = "cozis",
        .password = "mysecret",
        .host_name = "example.com",
        .host_ipv4 = "127.0.0.1",
        .host_ipv6 = "::0",
        .port = 8080,
        .abs_path = "/index.html",
        .rel_path = "index.html",
        .query = "name=francesco&date=today",
        .fragment = "intro"
    };
    xurl_t exp;
    char buffer[1024];
    for (size_t i = 0, len; generate_uri(i, &config, buffer, &len, sizeof(buffer), &exp); i++) {
        xurl_t out;
        bool res = xurl_parse(buffer, len, &out);

        char error[512];
        FILE *errfp = fmemopen(error, sizeof(error), "w");
        if (!res)
            fprintf(stderr, "\n" ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET " %s\n  Parsing failed\n", buffer);
        else if (!compare_results(&out, &exp, errfp)) {
            fflush(errfp);
            fprintf(stderr, "\n" ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET " %s\n%s", buffer, error);
        } else
            fprintf(stderr, ANSI_COLOR_GREEN "PASSED" ANSI_COLOR_RESET " %s\n", buffer);
        fclose(errfp);
    }
    return 0;
}
