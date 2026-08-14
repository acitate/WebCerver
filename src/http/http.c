#include <stdio.h>
#include <stdlib.h>
#include "sds.h"
#include "http.h"
#include <string.h>
#include <stddef.h>


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


HttpMethod lookup_method(const sds token)
{
    for (unsigned long i = 0; i < sizeof(METHOD_TABLE)/sizeof(*METHOD_TABLE); i++) {
        if (strcmp(METHOD_TABLE[i].name, token) == 0){
            return METHOD_TABLE[i].method;
        }
    }
    return HTTP_METHOD_UNDEFINED;
}


HttpParseStatus parse_request_line(sds request_line, HttpRequest *req)
{
    int token_count;
    sds *tokens = sdssplitlen(request_line, sdslen(request_line), " ", 1, &token_count);

    if (token_count != 3)
        return HTTP_PARSE_ERR_MALFORMED_REQUEST_LINE;

    if (strncmp(tokens[1], "/", 1) != 0)
        return HTTP_PARSE_ERR_MALFORMED_REQUEST_LINE;

    if (sdslen(tokens[1]) > MAX_URI_LEN)
        return HTTP_PARSE_ERR_URI_TOO_LONG;
    
    req->method = lookup_method(tokens[0]);
    req->path = tokens[1];
    req->version = tokens[2];

    return HTTP_PARSE_OK;
}


HttpParseStatus parse_headers(sds headers, HttpRequest *req)
{
    int line_count;
    sds *lines = sdssplitlen(headers, sdslen(headers), "\r\n", 2, &line_count);

    if (line_count > MAX_HEADERS)
        return HTTP_PARSE_ERR_TOO_MANY_HEADERS;

    for (int i = 0; i < line_count; i++) {

        if (sdslen(lines[i]) > MAX_HEADER_LEN)
            return HTTP_PARSE_ERR_HEADER_TOO_LONG;

        size_t line_len = sdslen(lines[i]);
        size_t colon_idx = find_first(lines[i], line_len, ":", 1);

        if (colon_idx == SIZE_MAX)
            return HTTP_PARSE_ERR_MALFORMED_HEADER;

        HttpHeader header;

        header.name = sdsnew(lines[i]);
        sdsrange(header.name, 0, colon_idx - 1);
        sdstrim(header.name, " ");

        header.value = sdsnew(lines[i]);
        sdsrange(header.value, colon_idx + 1, line_len);
        sdstrim(header.value, " ");

        req->headers[i] = header; 
    }
    req->header_count = line_count;

    sdsfreesplitres(lines, line_count);

    return HTTP_PARSE_OK;
}


HttpParseStatus parse_body(sds body, HttpRequest *req)
{
    req->body_len = sdslen(body);
    req->body = body;

    return HTTP_PARSE_OK;
}


HttpParseStatus http_parse_request(const sds raw, size_t len, HttpRequest *out)
{
    sds request_line, headers, body;
    split_request(raw, len, &request_line, &headers, &body);
    
    TRY(parse_request_line(request_line, out));
    TRY(parse_headers(headers, out));
    parse_body(body, out);

    
    return HTTP_PARSE_OK;
}


void http_build_response(HttpResponse resp, sds *resp_buf, size_t *resp_len)
{
    sds status_line = sdscatprintf(sdsempty(), "HTTP/1.1 %i %s\r\n", resp.status_code, resp.reason_phrase);
    sds headers_str = sdsempty();
    sds body = sdscatprintf(sdsempty(), "%s", resp.body);
    
    for (size_t hc = 0; hc < resp.header_count; hc++)
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


sds get_mime_type(sds filename)
{
    sds ext = strrchr(filename, '.');

    if (ext == NULL) {
        return DEFAULT_MIME_TYPE;
    }
    
    ext++;

    sdstolower(ext);

    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) { return "text/html"; }
    if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0) { return "image/jpg"; }
    if (strcmp(ext, "css") == 0) { return "text/css"; }
    if (strcmp(ext, "js") == 0) { return "application/javascript"; }
    if (strcmp(ext, "json") == 0) { return "application/json"; }
    if (strcmp(ext, "txt") == 0) { return "text/plain"; }
    if (strcmp(ext, "gif") == 0) { return "image/gif"; }
    if (strcmp(ext, "png") == 0) { return "image/png"; }

    return DEFAULT_MIME_TYPE;
}


const sds http_reason_phrase(int status_code) 
{
    switch (status_code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 414: return "URI Too Long";
        case 431: return "Request Header Fields Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 505: return "HTTP Version Not Supported";
        default:  return "Error";
    }
}