
all: test parse-url

test: tests/test.c tests/test_url.c tests/test_ipv4.c tests/test_ipv6.c  xurl.c
	gcc tests/test.c tests/test_url.c tests/test_ipv4.c tests/test_ipv6.c xurl.c -o test -Wall -Wextra -g -fprofile-arcs -ftest-coverage -fsanitize=address

parse-url: cli.c xurl.c
	gcc cli.c xurl.c -o parse-url -Wall -Wextra -DXURL_ZEROTERMINATE=1 -fsanitize=address -g

clean:
	rm *.gcda *.gcno *.gcov parse-url test