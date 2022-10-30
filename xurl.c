#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "xurl.h"

// [<schema> : ] // [ <username> [ : <password> ] @ ] { <name> | <IPv4> | "[" <IPv5> "]" } [ : <port> ] [ </path> ] [ ? <query> ] [ # <fragment> ]

static bool is_unreserved(char c)
{
    return isalpha(c) || isdigit(c) || c == '-' 
        || c == '.' || c == '_' || c == '~';
}

static bool is_subdelim(char c)
{
    return c == '!' || c == '$' ||
           c == '&' || c == '\'' ||
           c == '(' || c == ')' ||
           c == '*' || c == '+' ||
           c == ',' || c == ';' ||
           c == '=';
}

static bool is_pchar(char c)
{
    return is_unreserved(c) || is_subdelim(c) 
        || c == ':' || c == '@';
}

static bool is_schema_first(char c)
{
    return isalpha(c);
}

static bool is_schema(char c)
{
    return isalpha(c) 
        || isdigit(c) 
        || c == '+' 
        || c == '-'
        || c == '.';
}

static bool parse_schema(XURL_INPUT_CONSTNESS char *src, 
                         size_t len, size_t *i, 
                         XURL_INPUT_CONSTNESS char **schema, 
                         size_t *schema_len)
{
    size_t peek = *i;

    bool no_schema;
    size_t schema_offset;
    size_t schema_length;
    
    if (peek == len || !is_schema_first(src[peek]))
        no_schema = true;
    else {
        schema_offset = peek;
        do
            peek++;
        while (peek < len && is_schema(src[peek]));
        schema_length = peek - schema_offset;
        if (peek == len || src[peek] != ':')
            no_schema = true;
        else {
            no_schema = false;
            peek++; // Skip the ':'
        }
    }

    if (no_schema) {
        *schema = NULL;
        *schema_len = 0;
    } else {
        *schema = src + schema_offset;
        *schema_len = schema_length;
        *i = peek;
    }
    return true;
}

static bool is_username(char c)
{
    return is_unreserved(c) || is_subdelim(c);
}

static bool is_username_first(char c)
{
    return is_username(c);
}

static bool is_password(char c)
{
    return is_username(c);
}

static bool is_password_first(char c)
{
    return is_password(c);
}

static bool parse_userinfo(XURL_INPUT_CONSTNESS char *src, 
                           size_t len, size_t *i, 
                           xurl_userinfo *userinfo)
{
    bool no_username;
    bool no_password;
    size_t username_offset;
    size_t username_length;
    size_t password_offset;
    size_t password_length;

    size_t peek = *i;
    if (peek == len || !is_username_first(src[peek])) {
        no_username = true;
        no_password = true;
    } else {
        username_offset = peek;
        do
            peek++;
        while (peek < len && is_username(src[peek]));
        username_length = peek - username_offset;
        
        if (peek+1 < len && src[peek] == ':' && is_password_first(src[peek+1])) {
            peek++; // Skip the ':'
            password_offset = peek;
            do
                peek++;
            while (peek < len && is_password(src[peek]));
            password_length = peek - password_offset;

            if (peek == len || src[peek] != '@') {
                no_username = true;
                no_password = true;
            } else {
                no_username = false;
                no_password = false;
                peek++; // Skip the '@'
            }

        } else if (peek == len || src[peek] != '@') {
            no_username = true;
            no_password = true;
        } else {
            no_username = false;
            no_password = true;
            peek++; // Skip the '@'
        }
    }
    if (no_username) {
        userinfo->username = NULL;
        userinfo->username_len = 0;
    } else {
        userinfo->username = src + username_offset;
        userinfo->username_len = username_length;
        *i = peek;
    }
    if (no_password) {
        userinfo->password = NULL;
        userinfo->password_len = 0;
    } else {
        userinfo->password = src + password_offset;
        userinfo->password_len = password_length;
    }
    return true;
}

static bool parse_ipv4_byte(const char *src, size_t len,
                            size_t *i, uint8_t *out)
{
    size_t k = *i;
    uint8_t byte = 0;

    if (k == len || !isdigit(src[k]))
        return false;

    // TODO: Don't allow arbitrary sequence of
    //       0s at the start.
    do {
        int d = src[k] - '0';
        if (byte > (UINT8_MAX - d) / 10)
            return false; /* Overflow */
        byte = byte * 10 + d;
        k++;
    } while (k < len && isdigit(src[k]));

    *i = k;
    *out = byte;
    return true;
}

static bool parse_ipv4(const char *src, size_t len, 
                       size_t *i, uint32_t *ipv4)
{
    uint8_t unpacked_ipv4[4];

    for (int u = 0; u < 3; u++) {
        if (!parse_ipv4_byte(src, len, i, unpacked_ipv4 + u))
            return false;

        if (*i == len || src[*i] != '.')
            return false;
        (*i)++; // Skip the dot
    }
    if (!parse_ipv4_byte(src, len, i, unpacked_ipv4 + 3))
        return false;

    uint32_t packed_ipv4 = 
        ((uint32_t) unpacked_ipv4[0] << 24) |
        ((uint32_t) unpacked_ipv4[1] << 16) |
        ((uint32_t) unpacked_ipv4[2] <<  8) |
        ((uint32_t) unpacked_ipv4[3] <<  0);

    *ipv4 = packed_ipv4;
    return true;
}

static bool parse_port(const char *src, size_t len, 
                       size_t *i, bool *no_port, 
                       uint16_t *port)
{
    size_t k = *i;

    if (k+1 < len && src[k] == ':' && isdigit(src[k+1])) {

        k++; // Skip the ':'
        
        uint16_t p = 0;
        do {
            int d = src[k] - '0';
            if (p > (UINT16_MAX - d) / 10)
                break;
            p = p * 10 + d;
            k++;
        } while (k < len && isdigit(src[k]));
        
        *port = p;
        *no_port = false;
    } else {
        *port = 0;
        *no_port = true;
    }

    *i = k;
    return true;
}

static bool is_hostname(char c)
{
    return is_unreserved(c) || is_subdelim(c);
}

static bool is_hostname_first(char c)
{
    return is_hostname(c);
}

static bool parse_host(XURL_INPUT_CONSTNESS char *src, 
                       size_t len, size_t *i, 
                       xurl_host *host)
{
    size_t k = *i;
    if (k == len)
        return false;
    
    if (src[k] == '[') {
        
        // IPv6
        return false; /* Not implemented yet */

    } else {

        uint32_t ipv4;
        bool  is_ipv4;

        if (k < len && isdigit(src[k]))
            is_ipv4 = parse_ipv4(src, len, &k, &ipv4);
        else
            is_ipv4 = false;

        if (is_ipv4) {
            host->ipv4 = ipv4;
            host->mode = XURL_HOSTMODE_IPV4;
        } else {

            if (k == len || !is_hostname_first(src[k]))
                return false;
            size_t name_offset = k;
            do 
                k++;
            while (k < len && is_hostname(src[k]));
            size_t name_length = k - name_offset;

            host->mode = XURL_HOSTMODE_NAME;
            host->name = src + name_offset;
            host->name_len = name_length;
        }
    }

    uint16_t port;
    bool  no_port;
    if (!parse_port(src, len, &k, &no_port, &port))
        return false;
    host->port = port;
    host->no_port = no_port;
    *i = k;
    return true;
}

static bool follows_authority(const char *src, size_t len, 
                              size_t i)
{
    return i+1 < len && src[i] == '/' && src[i+1] == '/';
}

static bool parse_path(XURL_INPUT_CONSTNESS char *src, 
                       size_t len, size_t *i, 
                       XURL_INPUT_CONSTNESS char **path,
                       size_t *path_len)
{
    size_t k = *i;

    size_t path_offset;
    size_t path_length;

    path_offset = k;

    if (k < len && src[k] == '/') 
        k++;

    while (k < len && is_pchar(src[k])) {
        do
            k++;
        while (k < len && is_pchar(src[k]));
        if (k == len || src[k] != '/')
            break;
        k++; // Skip the '/'
    }

    path_length = k - path_offset;

    *i = k;
    *path = src + path_offset;
    *path_len = path_length;
    return true;
}

static bool is_query(char c)
{
    return is_pchar(c) || c == '/' || c == '?';
}

static bool parse_query(XURL_INPUT_CONSTNESS char *src, 
                        size_t len, size_t *i, 
                        XURL_INPUT_CONSTNESS char **query, 
                        size_t *query_len)
{
    bool no_query;
    size_t query_offset;
    size_t query_length;

    size_t peek = *i;
    if (peek < len && src[peek] == '?') {
        peek++; // Skip the '?'
        query_offset = peek;
        do
            peek++;
        while (peek < len && is_query(src[peek]));
        query_length = peek - query_offset;
        no_query = false;
    } else {
        query_offset = 0;
        query_length = 0;
        no_query = true;
    }

    if (no_query) {
        *query = NULL;
        *query_len = 0;
    } else {
        *i = peek;
        *query = src + query_offset;
        *query_len = query_length;
    }
    return true;
}

static bool is_fragment(char c)
{
    return is_pchar(c) || c == '/';
}

static bool parse_fragment(XURL_INPUT_CONSTNESS char *src, 
                           size_t len, size_t *i, 
                           XURL_INPUT_CONSTNESS char **fragment,
                           size_t *fragment_len)
{
    bool no_fragment;
    size_t fragment_offset;
    size_t fragment_length;

    size_t peek = *i;
    if (peek < len && src[peek] == '#') {
        peek++; // Skip the '#'
        fragment_offset = peek;
        do
            peek++;
        while (peek < len && is_fragment(src[peek]));
        fragment_length = peek - fragment_offset;
        no_fragment = false;
    } else {
        fragment_offset = 0;
        fragment_length = 0;
        no_fragment = true;
    }

    if (no_fragment) {
        *fragment = NULL;
        *fragment_len = 0;
    } else {
        *i = peek;
        *fragment = src + fragment_offset;
        *fragment_len = fragment_length;
    }
    return true;
}

#if XURL_ZEROTERMINATE
typedef struct {
    char  *pool;
    size_t size;
    size_t used;
} alloc_t;

static char *get_some_memory(alloc_t *alloc, size_t count)
{
    if (alloc->used + count > alloc->size)
        return NULL;

    char *ptr = alloc->pool + alloc->used;
    alloc->used += count;

    return ptr;
}

static char *my_strdup(const char *src, size_t len, alloc_t *alloc)
{
    char *dst = get_some_memory(alloc, len+1);
    if (dst == NULL)
        return false;
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}
#endif

bool xurl_parse2(XURL_INPUT_CONSTNESS char *src, 
                 size_t len, size_t *i, xurl_t *url)
{
    size_t maybe;
    if (i == NULL) {
        maybe = 0;
        i = &maybe;
    }

    if (!parse_schema(src, len, i, &url->schema, 
                      &url->schema_len))
        return false;

    if (follows_authority(src, len, *i)) {

        *i += 2; // Skip the "//"
        
        if (!parse_userinfo(src, len, i, &url->userinfo))
            return false;

        if (!parse_host(src, len, i, &url->host))
            return false;

        if (*i < len && src[*i] == '/') {
            /* absolute path */
            if (!parse_path(src, len, i, &url->path, 
                            &url->path_len))
                return false;
        } else {
            url->path = NULL;
            url->path_len = 0;
        }

    } else {

        url->host.mode = XURL_HOSTMODE_NAME;
        url->host.name = NULL;
        url->host.no_port = true;
        url->host.port = 0;
        
        url->userinfo.username = NULL;
        url->userinfo.password = NULL;
        url->userinfo.username_len = 0;
        url->userinfo.password_len = 0;

        // TODO: Since there was no authority,
        //       the path is non optional.

        if (*i == len || src[*i] == '?' || src[*i] == '#')
            return false; // Missing path

        if (!parse_path(src, len, i, &url->path, 
                        &url->path_len))
            return false;
    }

    if (!parse_query(src, len, i, &url->query, 
                     &url->query_len))
        return false;
    if (!parse_fragment(src, len, i, &url->fragment, 
                        &url->fragment_len))
        return false;

#if XURL_ZEROTERMINATE
    {
        alloc_t alloc = {
            .pool = url->buffer,
            .size = sizeof(url->buffer),
            .used = 0,
        };

        if (url->schema != NULL) {
            assert(url->schema[url->schema_len] == ':');
            url->schema[url->schema_len] = '\0';
        }

        char  *username = url->userinfo.username;
        size_t username_len = url->userinfo.username_len;
        if (username != NULL) {
            username[username_len] = '\0';
            char  *password = url->userinfo.password;
            size_t password_len = url->userinfo.password_len;
            if (password != NULL) {
                password[password_len] = '\0';
            }
        }

        if (url->host.mode == XURL_HOSTMODE_NAME) {
            if (url->host.no_port && (url->path != NULL || (url->query == NULL && url->fragment == NULL))) {
                
                url->host.name = my_strdup(url->host.name, url->host.name_len, &alloc);
                if (url->host.name == NULL)
                    return false;

            } else
                url->host.name[url->host.name_len] = '\0';
        }

        if (url->fragment != NULL) {
    
            // copy fragment to make it zero-terminated
            url->fragment = my_strdup(url->fragment, url->fragment_len, &alloc);
            if (url->fragment == NULL)
                return false;

            if (url->query != NULL)
                url->query[url->query_len] = '\0';
            
            if (url->path != NULL)
                url->path[url->path_len] = '\0';

        } else if (url->query != NULL) {

            // copy query to make it zero-terminated
            url->query = my_strdup(url->query, url->query_len, &alloc);
            if (url->query == NULL)
                return false;

            if (url->path != NULL)
                url->path[url->path_len] = '\0';

        } else if (url->path != NULL) {

            // copy path to make it zero-terminated
            url->path = my_strdup(url->path, url->path_len, &alloc);
            if (url->path == NULL)
                return false;
        }
    }
#endif
    return true;
}

bool xurl_parse(XURL_INPUT_CONSTNESS char *src, 
                size_t len, xurl_t *url)
{
    return xurl_parse2(src, len, NULL, url);
}
