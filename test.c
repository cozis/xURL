#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xurl.h"

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

int main(void)
{
    {
        xurl_t url_data;
        char  *url_text = "http://example.com";
        
        check(xurl_parse(url_text, strlen(url_text), &url_data) == true);
        
        check(!strncmp(url_data.schema, "http", url_data.schema_len));
        check(url_data.schema_len == 4);

        check(url_data.host.mode == XURL_HOSTMODE_NAME);
        check(!strncmp(url_data.host.name, "example.com", url_data.host.name_len));
        check(url_data.host.name_len == 11);
        check(url_data.host.no_port == true);
        check(url_data.host.port == 0);

        check(url_data.userinfo.username == NULL);
        check(url_data.userinfo.password == NULL);
        check(url_data.userinfo.username_len == 0);
        check(url_data.userinfo.password_len == 0);

        check(url_data.path == NULL);
        check(url_data.path_len == 0);

        check(url_data.query == NULL);
        check(url_data.query_len == 0);

        check(url_data.fragment == NULL);
        check(url_data.fragment_len == 0);
    
        printf("OK\n");
    }

    {
        xurl_t url_data;
        char  *url_text = "http://itsme:mario@example.com";
        
        check(xurl_parse(url_text, strlen(url_text), &url_data) == true);
        
        check(!strncmp(url_data.schema, "http", url_data.schema_len));
        check(url_data.schema_len == 4);

        check(url_data.host.mode == XURL_HOSTMODE_NAME);
        check(!strncmp(url_data.host.name, "example.com", url_data.host.name_len));
        check(url_data.host.name_len == 11);
        check(url_data.host.no_port == true);
        check(url_data.host.port == 0);
        
        check(!strncmp(url_data.userinfo.username, "itsme", url_data.userinfo.username_len));
        check(!strncmp(url_data.userinfo.password, "mario", url_data.userinfo.password_len));
        check(url_data.userinfo.username_len == sizeof("itsme")-1);
        check(url_data.userinfo.password_len == sizeof("mario")-1);

        check(url_data.path == NULL);
        check(url_data.path_len == 0);

        check(url_data.query == NULL);
        check(url_data.query_len == 0);

        check(url_data.fragment == NULL);
        check(url_data.fragment_len == 0);
    
        printf("OK\n");
    }

    {
        xurl_t url_data;
        char  *url_text = "http://itsme@example.com";
        
        check(xurl_parse(url_text, strlen(url_text), &url_data) == true);
        
        check(!strncmp(url_data.schema, "http", url_data.schema_len));
        check(url_data.schema_len == 4);

        check(url_data.host.mode == XURL_HOSTMODE_NAME);
        check(!strncmp(url_data.host.name, "example.com", url_data.host.name_len));
        check(url_data.host.name_len == 11);
        check(url_data.host.no_port == true);
        check(url_data.host.port == 0);
        
        check(!strncmp(url_data.userinfo.username, "itsme", url_data.userinfo.username_len));
        check(url_data.userinfo.password == NULL);
        check(url_data.userinfo.username_len == sizeof("itsme")-1);
        check(url_data.userinfo.password_len == 0);

        check(url_data.path == NULL);
        check(url_data.path_len == 0);

        check(url_data.query == NULL);
        check(url_data.query_len == 0);

        check(url_data.fragment == NULL);
        check(url_data.fragment_len == 0);
        
        printf("OK\n");
    }

    return 0;
}
