#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/http/http.h"
#include "sds.h"

static void test_parse_simple_get(void)
{
    const char *raw =
        "GET /hello HTTP/1.1\r\n"
        "\r\n";

    HttpRequest req = {0};

    HttpParseStatus status =
        http_parse_request(raw, strlen(raw), &req);

    assert(status == HTTP_PARSE_OK);
    
    assert(req.method == HTTP_METHOD_GET);
    assert(sdscmp(req.path, sdsnew("/hello")) == 0);
    assert(sdscmp(req.version, sdsnew("HTTP/1.1")) == 0);

    assert(req.header_count == 0);
    assert(req.body_len == 0);
    assert(sdscmp(sdsempty(), req.body) == 0);

    sdsfree(req.path);
    sdsfree(req.version);
}


static void test_parse_get_with_headers(void)
{
    const char *raw =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";

    HttpRequest req = {0};

    HttpParseStatus status =
        http_parse_request(raw, strlen(raw), &req);

    assert(status == HTTP_PARSE_OK);

    assert(req.method == HTTP_METHOD_GET);
    assert(sdscmp(req.path, sdsnew("/index.html")) == 0);
    assert(sdscmp(req.version, sdsnew("HTTP/1.1")) == 0);

    assert(req.header_count == 2);

    assert(sdscmp(req.headers[0].name, sdsnew("Host")) == 0);
    assert(sdscmp(req.headers[0].value, sdsnew("example.com")) == 0);


    assert(sdscmp(req.headers[1].name, sdsnew("Connection")) == 0);
    assert(sdscmp(req.headers[1].value, sdsnew("close")) == 0);

    assert(req.body_len == 0);
    assert(sdscmp(sdsempty(), req.body) == 0);

    sdsfree(req.path);
    sdsfree(req.version);

    for (size_t i = 0; i < req.header_count; i++) {
        sdsfree(req.headers[i].name);
        sdsfree(req.headers[i].value);
    }
}


static void test_parse_empty_path(void)
{
    const char *raw =
        "GET / HTTP/1.1\r\n"
        "\r\n";

    HttpRequest req = {0};

    HttpParseStatus status =
        http_parse_request(raw, strlen(raw), &req);

    assert(status == HTTP_PARSE_OK);
    assert(req.method == HTTP_METHOD_GET);
    assert(strcmp(req.path, "/") == 0);
    assert(strcmp(req.version, "HTTP/1.1") == 0);
    assert(req.header_count == 0);
    assert(req.body_len == 0);

    sdsfree(req.path);
    sdsfree(req.version);
}


static void test_malformed_request_line(void)
{
    const char *raw =
        "GET\r\n"
        "\r\n";

    HttpRequest req = {0};

    HttpParseStatus status =
        http_parse_request(raw, strlen(raw), &req);

    assert(status == HTTP_PARSE_ERR_MALFORMED_REQUEST_LINE);
}


static void test_unsupported_http_version(void)
{
    const char *raw =
        "GET / HTTP/2.0\r\n"
        "\r\n";

    HttpRequest req = {0};

    HttpParseStatus status =
        http_parse_request(raw, strlen(raw), &req);

    assert(status == HTTP_PARSE_ERR_UNSUPPORTED_VERSION);
}


static void test_malformed_header(void)
{
    const char *raw =
        "GET / HTTP/1.1\r\n"
        "Host example.com\r\n"
        "\r\n";

    HttpRequest req = {0};

    HttpParseStatus status =
        http_parse_request(raw, strlen(raw), &req);

    assert(status == HTTP_PARSE_ERR_MALFORMED_HEADER);
}


static void test_incomplete_request(void)
{
    const char *raw =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n";

    HttpRequest req = {0};

    HttpParseStatus status =
        http_parse_request(raw, strlen(raw), &req);

    assert(status == HTTP_PARSE_INCOMPLETE);
}


int main(void)
{
    test_parse_simple_get();
    test_parse_get_with_headers();
    test_parse_empty_path();
    test_malformed_request_line();
    test_unsupported_http_version();
    test_malformed_header();
    test_incomplete_request();

    printf("All tests passed.\n");

    return 0;
}