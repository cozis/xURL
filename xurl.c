#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "xurl.h"

// [<schema> : ] // [ <username> [ : <password> ] @ ] { <name> | <IPv4> | "[" <IPv5> "]" } [ : <port> ] [ </path> ] [ ? <query> ] [ # <fragment> ]

static bool is_lower_alpha(char c)
{
    return c >= 'a' && c <= 'z';
}

static bool is_upper_alpha(char c)
{
    return c >= 'A' && c <= 'Z';
}

static bool is_alpha(char c)
{
    return is_upper_alpha(c)
        || is_lower_alpha(c);
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_hex_digit(char c)
{
    return is_digit(c) 
        || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
}

static bool is_unreserved(char c)
{
    return is_alpha(c) || is_digit(c) 
        || c == '-' || c == '.' 
        || c == '_' || c == '~';
}

static bool is_subdelim(char c)
{
    return c == '!' || c == '$' 
        || c == '&' || c == '\'' 
        || c == '(' || c == ')' 
        || c == '*' || c == '+' 
        || c == ',' || c == ';' 
        || c == '=';
}

static bool is_pchar(char c)
{
    return is_unreserved(c) 
        || is_subdelim(c) 
        || c == ':' || c == '@';
}

static bool is_schema_first(char c)
{
    return is_alpha(c);
}

static bool is_schema(char c)
{
    return is_alpha(c) 
        || is_digit(c) 
        || c == '+' 
        || c == '-'
        || c == '.';
}

/* Symbol: parse_schema 
 *   Parse the schema of an url, if there is one.
 *
 * Arguments:
 *          (in) src: The source string.
 *
 *               len: The number of bytes pointed by [src].
 *
 *      (in/out)   i: Offset from where the parsing should
 *                    start. When the function returns, its
 *                    value is incremented by the number of
 *                    parsed bytes (therefore pointing to the
 *                    first non-parsed byte). If the url in
 *                    the source has no schema, it's value
 *                    is left unchanged.
 *
 *      (out) schema: Non zero-terminated string containing
 *                    the parsed schema. If the url didn't
 *                    have a schema, it's set to NULL.
 *
 *  (out) schema_len: Length of the parsed schema, or 0 if
 *                    there wasn't one.
 *
 * Returns:
 *   - [i] is incremented by the number of parsed bytes.
 *
 *   - [schema] points to the schema string (or NULL if
 *     there wasn't one).
 *
 *   - [schema_len] is the length of the string pointed
 *     by [schema], or 0 if there was no schema.
 *
 * Notes:
 *   - This function can never fail.
 */
static void parse_schema(XURL_INPUT_CONSTNESS char *src, 
                         size_t len, size_t *i, 
                         XURL_INPUT_CONSTNESS char **schema, 
                         size_t *schema_len)
{
    size_t peek = *i; // Local cursor

    bool no_schema;
    size_t schema_offset;
    size_t schema_length;
    
    // If there is a schema, set [no_schema] to
    // [false], [schema_offset] to the start of
    // the schema substring (relative to [src])
    // and [schema_length] to its length.
    {
        if (peek == len || !is_schema_first(src[peek]))
            // The first character can't be the start
            // of a schema.
            no_schema = true;
        else {
            // The current character is a valid start
            // for a schema. We'll assume it is until
            // proven otherwise.

            // Keep track of the current posizion, then
            // consume all characters that compose the
            // schema.
            schema_offset = peek;
            do
                peek++;
            while (peek < len && is_schema(src[peek]));
            schema_length = peek - schema_offset;

            // If the character following the schema
            // isn't a ':', it wasn't a schema after
            // all.
            if (peek == len || src[peek] != ':')
                no_schema = true;
            else {
                no_schema = false;
                peek++; // Skip the ':'
            }
        }
    }

    // Update the output parameters.
    if (no_schema) {
        *schema = NULL;
        *schema_len = 0;
        // Don't unpdate [i]
    } else {
        *schema = src + schema_offset;
        *schema_len = schema_length;
        *i = peek; // Commit changes.
    }
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

/* Symbol: parse_userinfo
 *   Parse the userinfo component of an URL,
 *   if there is one.
 *
 *   The userinfo component is a subcomponent
 *   of the authority. It contains a username
 *   and, optionally, a password:
 *
 *     http://username@example.com
 *     http://username:password@example.com
 *
 *   This functions parses the portion that
 *   goes from the byte following "//" to 
 *   the '@'.
 *
 * Arguments:
 *         (in) src: The source string.
 *
 *              len: The number of bytes pointed by [src]
 *
 *       (in/out) i: Offset from where the parsing should
 *                   start. When the function returns, its
 *                   value is incremented by the number of
 *                   parsed bytes (therefore pointing to the
 *                   first non-parsed byte). If the url in
 *                   the source has no userinfo, it's value
 *                   is left unchanged.
 *
 *   (out) userinfo: The result of the parsing. 
 * 
 * Returns:
 *   - [i] is incremented by the number of parsed bytes.
 *
 *   - [userinfo] contains the parsed username and
 *     password. These strings are not zero-terminated.
 *     If there was no userinfo component, then
 *     [userinfo.username] and [userinfo.password] are
 *     NULL and [userinfo.username_len] and
 *     [userinfo.password_len] are 0. If the username
 *     is specified but the password isn't, then only
 *     [userinfo.password] is NULL and 
 *     [userinfo.password_len] is 0.
 *
 * Notes:
 *   - This function can never fail.
 */
static void parse_userinfo(XURL_INPUT_CONSTNESS char *src, 
                           size_t len, size_t *i, 
                           xurl_userinfo *userinfo)
{
    size_t peek = *i; // Local cursor

    bool no_username;
    bool no_password;
    size_t username_offset;
    size_t username_length;
    size_t password_offset;
    size_t password_length;

    // If the userinfo subcomponent is present,
    // set [no_username] to false, [username_offset]
    // to the offset of the username relative to 
    // [src] and [username_length] to its length. 
    // If the password was also specified, do the
    // same.  
    {
        if (peek == len || !is_username_first(src[peek])) {
            // The first character can't be the first
            // of an username, therefore there's no
            // userinfo subcomponent.
            no_username = true;
            no_password = true;
        } else {

            // The first character is a valid start
            // for an username, therefore we'll assume
            // that there's a userinfo subcomponent
            // until proven otherwise.

            // Scan the username while keeping track
            // of it's offset and length.
            username_offset = peek;
            do
                peek++;
            while (peek < len && is_username(src[peek]));
            username_length = peek - username_offset;
            
            // If the username is followed by a ':' and
            // a valid password character, we also expect 
            // a password.
            if (peek+1 < len && src[peek] == ':' && is_password_first(src[peek+1])) {
                
                peek++; // Skip the ':'
                
                // Scan the password while keeping track
                // of it's offset and length.
                password_offset = peek;
                do
                    peek++;
                while (peek < len && is_password(src[peek]));
                password_length = peek - password_offset;

                if (peek == len || src[peek] != '@') {
                    // If the password isn't followed by a '@',
                    // then this wasn't a userinfo subcomponent
                    // after all.
                    no_username = true;
                    no_password = true;
                } else {
                    // All done. 
                    no_username = false;
                    no_password = false;
                    peek++; // Skip the '@'
                }

            } else if (peek == len || src[peek] != '@') {
                // Since no password was specified, if the 
                // username isn't followed by a '@', then 
                // this wasn't a userinfo subcomponent after 
                // all.
                no_username = true;
                no_password = true;
            } else {
                // All done.
                no_username = false;
                no_password = true;
                peek++; // Skip the '@'
            }
        }
    }

    // Update the output parameters.
    if (no_username) {
        assert(no_password);
        userinfo->username = NULL;
        userinfo->username_len = 0;
        userinfo->password = NULL;
        userinfo->password_len = 0;
        // Don't update [i]
    } else {
        
        userinfo->username = src + username_offset;
        userinfo->username_len = username_length;

        if (no_password) {
            userinfo->password = NULL;
            userinfo->password_len = 0;
        } else {
            userinfo->password = src + password_offset;
            userinfo->password_len = password_length;
        }

        *i = peek; // Commit changes.
    }
}

static bool parse_ipv4_byte(const char *src, size_t len,
                            size_t *i, uint8_t *out)
{
    size_t peek = *i;

    // If the cursor refers to a digit, then
    // there's a byte to parse.
    bool start_with_digit = (peek < len && is_digit(src[peek]));

    if (start_with_digit) {

        uint8_t byte = 0;
    
        // TODO: Don't allow arbitrary sequence of
        //       0s at the start.
        do {
            int d = src[peek] - '0';
            if (byte > (UINT8_MAX - d) / 10)
                break; // Overflow! This digit isn't part of the byte.
            byte = byte * 10 + d;
            peek++;
        } while (peek < len && is_digit(src[peek]));

        *i = peek;
        *out = byte;
    }

    return start_with_digit;
}

static uint32_t pack(uint8_t *bytes)
{
    return ((uint32_t) bytes[0] << 24)
         | ((uint32_t) bytes[1] << 16)
         | ((uint32_t) bytes[2] <<  8)
         | ((uint32_t) bytes[3] <<  0);
}

static bool parse_ipv4(const char *src, size_t len, 
                       size_t *i, uint32_t *ipv4)
{
    size_t peek = *i;
    uint8_t unpacked_ipv4[4];

    for (int u = 0; u < 3; u++) {

        if (!parse_ipv4_byte(src, len, &peek, unpacked_ipv4 + u))
            return false;

        if (peek == len || src[peek] != '.')
            return false;

        peek++; // Skip the dot
    }
    if (!parse_ipv4_byte(src, len, &peek, unpacked_ipv4 + 3))
        return false;

    *ipv4 = pack(unpacked_ipv4);
    *i = peek;
    return true;
}

static int hex_digit_to_int(char c)
{
    assert(is_hex_digit(c));
    
    if (is_lower_alpha(c))
        return c - 'a' + 10;
    
    if (is_upper_alpha(c))
        return c - 'A' + 10;
    
    assert(is_digit(c));
    return c - '0';
}

static bool parse_ipv6_word(const char *src, size_t len,
                            size_t *i, uint16_t *out)
{
    size_t peek = *i;

    // If there's at least one hex digit at the
    // current position, then we can parse a word
    // for sure.
    bool start_with_hex_digit = (peek < len && is_hex_digit(src[peek]));

    if (start_with_hex_digit) {

        uint16_t word = 0;

        // TODO: Don't allow arbitrary sequence of
        //       0s at the start.
        do {
            
            int d = hex_digit_to_int(src[peek]);
            if (word > (UINT16_MAX - d) / 16)
                break; // Overflow! This hex digit isn't part of the word.
            
            word = word * 16 + d;
            
            peek++;

        } while (peek < len && is_hex_digit(src[peek]));

        *i = peek;
        *out = word;
    }

    return start_with_hex_digit;
}

static bool parse_ipv6(const char *src, size_t len,
                       size_t *i, uint16_t ipv6[static 8])
{
    size_t k = *i;

    uint16_t tail[8];
    size_t head_count = 0;
    size_t tail_count = 0;

    if (k+1 < len && src[k] == ':' && src[k+1] == ':')
        k += 2;
    else {
        while (1) {

            uint16_t word;
            if (!parse_ipv6_word(src, len, &k, &word))
                return false;

            ipv6[head_count++] = word;
            
            if (head_count == 8)
                break;
            
            if (k == len || src[k] != ':')
                return false;
            k++; // Skip the ':'

            if (k < len && src[k] == ':') {
                k++;
                break;
            }
        }
    }

    if (head_count + tail_count < 8) {
        while (k < len && is_hex_digit(src[k])) {

            // We know the current character is a
            // hex digit, therefore [parse_ipv6_word]
            // won't fail.
            uint16_t word;
            (void) parse_ipv6_word(src, len, &k, &word);

            tail[tail_count++] = word;
            
            if (head_count + tail_count == 7)
                break;
            
            if (k == len || src[k] != ':')
                break;
            k++; // Skip the ':'
        }
    }

    for (size_t p = 0; p < 8 - head_count - tail_count; p++)
        ipv6[head_count + p] = 0;

    for (size_t p = 0; p < tail_count; p++)
        ipv6[8 - tail_count + p] = tail[p];

    *i = k;
    return true;
}

static void parse_port(const char *src, size_t len, 
                       size_t *i, bool *no_port, 
                       uint16_t *port)
{
    size_t k = *i;

    if (k+1 < len && src[k] == ':' && is_digit(src[k+1])) {

        k++; // Skip the ':'
        
        uint16_t p = 0;
        do {
            int d = src[k] - '0';
            if (p > (UINT16_MAX - d) / 10)
                break;
            p = p * 10 + d;
            k++;
        } while (k < len && is_digit(src[k]));
        
        *port = p;
        *no_port = false;
    } else {
        *port = 0;
        *no_port = true;
    }

    *i = k;
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

        k++; // Skip the '['
        
        // IPv6
        if (!parse_ipv6(src, len, &k, host->ipv6))
            return false;

        if (k == len || src[k] != ']')
            return false;
        k++; // Skip the ']'

        host->mode = XURL_HOSTMODE_IPV6;

    } else {

        uint32_t ipv4;
        bool  is_ipv4;

        if (is_digit(src[k]))
            is_ipv4 = parse_ipv4(src, len, &k, &ipv4);
        else
            is_ipv4 = false;

        if (is_ipv4) {
            host->ipv4 = ipv4;
            host->mode = XURL_HOSTMODE_IPV4;
        } else {

            if (!is_hostname_first(src[k]))
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
    parse_port(src, len, &k, &no_port, &port);
    host->port = port;
    host->no_port = no_port;
    *i = k;
    return true;
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
    else {
        if (k == len || !is_pchar(src[k]))
            return false;
    }

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

static void parse_query(XURL_INPUT_CONSTNESS char *src, 
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
        while (peek < len && is_query(src[peek]))
            peek++;
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
}

static bool is_fragment(char c)
{
    return is_pchar(c) || c == '/';
}

static void parse_fragment(XURL_INPUT_CONSTNESS char *src, 
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
        while (peek < len && is_fragment(src[peek]))
            peek++;
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

static bool follows_authority(const char *src, size_t len, 
                              size_t i)
{
    return i+1 < len 
        && src[i] == '/' 
        && src[i+1] == '/';
}

bool xurl_parse2(XURL_INPUT_CONSTNESS char *src, 
                 size_t len, size_t *i, xurl_t *url)
{
    size_t maybe;
    if (i == NULL) {
        maybe = 0;
        i = &maybe;
    }

    parse_schema(src, len, i, 
                 &url->schema, 
                 &url->schema_len);

    if (follows_authority(src, len, *i)) {

        *i += 2; // Skip the "//"
        
        parse_userinfo(src, len, i, &url->userinfo);

        if (!parse_host(src, len, i, &url->host))
            return false;

        if (*i < len && src[*i] == '/') {
            /* absolute path */

            // The parsing of the path can't fail 
            // because we already know there's at
            // leat a '/' for it.
            (void) parse_path(src, len, i, &url->path, &url->path_len);
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

        if (!parse_path(src, len, i, &url->path, &url->path_len))
            return false;
    }

    parse_query(src, len, i, &url->query, &url->query_len);
    parse_fragment(src, len, i, &url->fragment, &url->fragment_len);

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
            if (password != NULL)
                password[password_len] = '\0';
            
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
    size_t i = 0;
    bool result = xurl_parse2(src, len, &i, url);
    return result && i == len;
}

bool xurl_parse_ipv4(const char *src, size_t len, 
                     uint32_t *out)
{
    size_t i = 0;
    return parse_ipv4(src, len, &i, out);
}

bool xurl_parse_ipv6(const char *src, size_t len, 
                     uint16_t out[8])
{
    size_t i = 0;
    return parse_ipv6(src, len, &i, out);
}