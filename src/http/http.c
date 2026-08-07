#include <stdio.h>
#include <stdlib.h>
#include "sds.h"
#include "http.h"
#include <string.h>
#include <stddef.h>


static const struct { const char *name; HttpMethod method; } METHOD_TABLE[] = {
    { "GET", HTTP_METHOD_GET },
};



size_t find_first(const char *str, size_t str_len, const char *sub_str, size_t sub_str_len)
{
    // Finds first occurrence of `sub_str` in `str` and returns its index.
    if (sub_str_len == 0) return 0;
    if (sub_str_len > str_len) return SIZE_MAX;

    for (size_t i = 0; i <= str_len - sub_str_len; i++) {
        size_t j = 0;
        while (j < sub_str_len && str[i + j] == sub_str[j]) j++;
        if (j == sub_str_len) return i;
    }
    return SIZE_MAX;
}


void split_request(sds raw, size_t raw_len, sds *req_line, sds *headers, sds *body)
{
    // Split raw request string to request line, Headers and body. 
    size_t idx = find_first(raw, raw_len, "\r\n\r\n", 4); // First occurrence of `\r\n\r\n` separating headers from body
    size_t line_end = find_first(raw, raw_len, "\r\n", 2); // First occurrence of `\r\n` signifying the end of request line
    
    *body = sdsnew(raw);
    sdsrange(*body, idx + 4, raw_len);

    *req_line = sdsnew(raw);
    sdsrange(*req_line, 0, line_end);

    *headers = sdsnew(raw);
    sdsrange(*headers, line_end + 2, idx);
}


HttpMethod lookup_method(const char *token)
{
    for (unsigned long i = 0; i < sizeof(METHOD_TABLE)/sizeof(*METHOD_TABLE); i++) {
        if (strcmp(METHOD_TABLE[i].name, token) == 0){
            return METHOD_TABLE[i].method;
        }
    }
    return HTTP_METHOD_UNDEFINED;
}


void parse_request_line(sds request_line, HttpRequest *req)
{
    int token_count;
    sds *tokens = sdssplitlen(request_line, sdslen(request_line), " ", 1, &token_count);

    req->method = lookup_method(tokens[0]);
    // strcpy(req->path, tokens[1]);
    req->path = tokens[1];
    // strcpy(req->versrion, tokens[2]);
    req->version = tokens[2];
}


void parse_headers(sds headers, HttpRequest *req)
{
    int line_count;
    sds *lines = sdssplitlen(headers, sdslen(headers), "\r\n", 2, &line_count);

    for (int i = 0; i < line_count; i++) {
        HttpHeader header;
        int _; 
        sds *tokens = sdssplitlen(lines[i], sdslen(lines[i]), ": ", 2, &_);

        header.name = tokens[0];
        header.value = tokens[1];
        // strcpy(header.name, tokens[0]);
        // strcpy(header.value, tokens[1]);

        req->headers[i] = header; 
    }
    req->header_count = line_count;
}


void parse_body(sds body, HttpRequest *req)
{
    req->body_len = sdslen(body);
    // req->body = malloc(req->body_len);
    req->body = body;
    // strcpy(req->body, body);
}


void http_parse_request(const sds raw, size_t len, HttpRequest *out)
{
    sds request_line, headers, body;
    split_request(raw, len, &request_line, &headers, &body);
    
    parse_request_line(request_line, out);
    parse_headers(headers, out);
    parse_body(body, out);
}


void http_build_response(HttpResponse resp, sds *resp_buf, size_t *resp_len)
{
    sds status_line = sdscatprintf(sdsempty(), "HTTP/1.1 %i %s\r\n", resp.status_code, resp.reason_phrase);
    sds headers_str = sdsempty();
    sds body = sdscatprintf(sdsempty(), "%s", resp.body);
    
    for (int hc = 0; hc < resp.header_count; hc++)
    {
        headers_str = sdscatprintf(headers_str, "%s: %s\r\n", resp.headers[hc].name, resp.headers[hc].value);
    }

    *resp_buf = sdscatprintf(sdsempty(), "%s%s\r\n", status_line, headers_str);
    *resp_buf = sdscatprintf(*resp_buf, "%s", body);
    *resp_len = sdslen(*resp_buf);

    sdsfree(status_line);
    sdsfree(headers_str);
    sdsfree(body);
}


void http_response_404(sds *resp_buf, size_t *resp_len)
{
    sds status_line = sdscatprintf(sdsempty(), "HTTP/1.1 %i %s\r\n", 404, "NOT FOUND");
    sds headers_str = sdsnew("Connection: close\r\nContent-Length: 21\r\nContent-Type: text/plain\r\n");
    sds body = sdsnew("Error 404: Not Found!");

    *resp_buf = sdscatprintf(sdsempty(), "%s%s\r\n%s", status_line, headers_str, body);
    *resp_len = sdslen(*resp_buf);
}