#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include "xurl.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <url>", argv[0]);
        return -1;
    }
    xurl_t result;
    if (xurl_parse(argv[1], strlen(argv[1]), &result)) {
        printf("SCHEMA  \t%s\n",   result.schema != NULL ? result.schema   : "---");
        printf("USERNAME\t%s\n", result.userinfo.username != NULL ? result.userinfo.username : "---");
        printf("PASSWORD\t%s\n", result.userinfo.password != NULL ? result.userinfo.password : "---");

        char buffer[32];
        switch (result.host.mode) {
            
            case XURL_HOSTMODE_NAME: 
            printf("HOST    \t%s\n", result.host.name ? result.host.name : "---"); 
            break;
            
            case XURL_HOSTMODE_IPV4: 
            inet_ntop(AF_INET, &result.host.ipv4, buffer, sizeof(buffer));
            printf("HOST (IPV4)\t%s\n", buffer); 
            break;
            
            case XURL_HOSTMODE_IPV6: 
            inet_ntop(AF_INET6, &result.host.ipv6, buffer, sizeof(buffer));
            printf("HOST (IPV6)\t%s\n", buffer); 
            break;
        }
        if (result.host.no_port) 
            printf("PORT    \t---\n");
        else 
            printf("PORT    \t%d\n", result.host.port);
        printf("PATH    \t%s\n",     result.path     != NULL ? result.path     : "---");
        printf("QUERY   \t%s\n",    result.query    != NULL ? result.query    : "---");
        printf("FRAGMENT\t%s\n", result.fragment != NULL ? result.fragment : "---");
        return 0;
    } else {
        fprintf(stderr, "Failed");
        return -1;
    }
}