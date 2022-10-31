#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "test.h"
#include "xurl.h"

int test_ipv6(size_t *total, size_t *passed)
{
    const char *list[] = {
        "::",
        "0::",
        "::0",
        "A:B::",
        "A:B:C::",
        "A:B:C:D::",
        "A:B:C:D:E::",
        "A:B:C:D:E:F::",
        "A:B:C:D:E:F:0::",
        "A:B:C:D:E:F:0:1",
        "0::F",
        "a::",
        "b::",
        "c::",
        "d::",
        "e::",
        "f::"
    };

    for (size_t i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
        const char *input = list[i];

        uint16_t output[8];
        uint16_t expected_ipv6[8];
        if (1 == inet_pton(AF_INET6, input, expected_ipv6)) {

            for (size_t g = 0; g < 8; g++)
                expected_ipv6[g] = ntohs(expected_ipv6[g]);

            /* Expected success */

            if (!xurl_parse_ipv6(input, strlen(input), output)) {
                fprintf(stderr, ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET "%s\n", input);
                fprintf(stderr, "  Parsing failed\n");
            } else if (memcmp(expected_ipv6, output, 16)) {
                fprintf(stderr, ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET "%s\n", input);
                fprintf(stderr, "  IPv6 doesn't match\n");
                fprintf(stderr, "    expected %.4X:%.4X:%.4X:%.4X:%.4X:%.4X:%.4X:%.4X\n", 
                        expected_ipv6[0], expected_ipv6[1], 
                        expected_ipv6[2], expected_ipv6[3], 
                        expected_ipv6[4], expected_ipv6[5], 
                        expected_ipv6[6], expected_ipv6[7]);
                fprintf(stderr, "    but got %.4X:%.4X:%.4X:%.4X:%.4X:%.4X:%.4X:%.4X\n", 
                        output[0], output[1], output[2], output[3], 
                        output[4], output[5], output[6], output[7]);
            } else {
                fprintf(stderr, ANSI_COLOR_GREEN "PASSED" ANSI_COLOR_RESET " %s\n", input);
                (*passed)++;
            }

        } else {
            /* Expected failure */

            if (xurl_parse_ipv6(input, strlen(input), output)) {
                fprintf(stderr, ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET "%s\n", input);
                fprintf(stderr, "  Parsing succeded unexpectedly\n");
            } else {
                fprintf(stderr, ANSI_COLOR_GREEN "PASSED" ANSI_COLOR_RESET " %s\n", input);
                (*passed)++;
            }
        }
        (*total)++;
    }
    return 0;
}