#include <stdio.h>
#include "test.h"

int main(void) {
    size_t total = 0;
    size_t passed = 0;
    test_ipv4(&total, &passed);
    test_ipv6(&total, &passed);
    test_url(&total, &passed);
    fprintf(stdout, "Passed %ld out of %ld tests\n", passed, total);
    return 0;
}