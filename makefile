all: test parse-url

test: test.c test_url.c test_ipv4.c test_ipv6.c  xurl.c
	gcc test.c test_url.c test_ipv4.c test_ipv6.c xurl.c -o test -Wall -Wextra -g -fprofile-arcs -ftest-coverage -fsanitize=address

parse-url: cli.c xurl.c
	gcc cli.c xurl.c -o parse-url -Wall -Wextra -DXURL_ZEROTERMINATE=1 -fsanitize=address -g
