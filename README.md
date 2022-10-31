# xURL
zero-allocation and (pretty much) zero-copy URL parsing library for C.

To use this library, copy `xurl.c` and `xurl.h` over to your source tree and compile them as they were your files (include `xurl.h` where you wan't to use `xurl_*` functions and pass `xurl.c` to your C compiles alongside your other C files)

By default xURL returns strings that arent zero-terminated. To change this behavious you can define `XURL_ZEROTERMINATE` as `1`.

Here are some cool properties of xURL:
* Never uses dynamic memory
* Never copies the input string while parsing it. All results are slices that refer to the original source. The only exception is when the user provides `XURL_ZEROTERMINATE` as `1`, in which case a minimum amount of copies is necessary to make the output strings zero-terminated.

# TODO
* handle percent-encoded URLs
* fuzz testing