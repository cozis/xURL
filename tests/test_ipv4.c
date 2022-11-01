#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "test.h"
#include "../xurl.h"

int test_ipv4(size_t *total, size_t *passed)
{
    const char *list[] = {
        "0.0.0.0",
        "0.0.0.1",
        "0.0.1.0",
        "0.1.0.0",
        "1.0.0.0",
        "255.255.255.255",
    };

    for (size_t i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
        const char *input = list[i];
        uint32_t expected_ipv4;
        if (1 == inet_pton(AF_INET, input, &expected_ipv4)) {

            expected_ipv4 = ntohl(expected_ipv4);

            /* Expected success */

            uint32_t output;
            if (!xurl_parse_ipv4(input, strlen(input), &output)) {
                fprintf(stderr, ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET " %s\n", input);
                fprintf(stderr, "  Parsing failed\n");
            } else if (expected_ipv4 != output) {
                fprintf(stderr, ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET " %s\n", input);
                fprintf(stderr, "  IP doesn't match\n");
            } else {
                fprintf(stderr, ANSI_COLOR_GREEN "PASSED" ANSI_COLOR_RESET " %s\n", input);
                (*passed)++;
            }

        } else {
            /* Expected failure */

            uint32_t output;
            if (xurl_parse_ipv4(input, strlen(input), &output)) {
                fprintf(stderr, ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET " %s\n", input);
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