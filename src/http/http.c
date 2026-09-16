#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sds.h"
#include "http.h"
#include <string.h>
#include <stddef.h>

/**
 * @brief Searches the string for the first occurrence of a specified value and returns the position of where it was found.
 * 
 * @param str String to search in.
 * @param str_len Length of the string to search in.
 * @param sub_str The value to search for.
 * @param sub_str_len Length of the value to search for.
 * @return size_t Index of the first character in the first occurrence of the value. `SIZE_MAX` if the value wasn't found in the string.
 */
size_t find_first(const char *str, size_t str_len, const char *sub_str, size_t sub_str_len)
{
    if (sub_str_len == 0) return 0;
    if (sub_str_len > str_len) return SIZE_MAX;

    for (size_t i = 0; i <= str_len - sub_str_len; i++) {
        size_t j = 0;
        while (j < sub_str_len && str[i + j] == sub_str[j]) j++;
        if (j == sub_str_len) return i;
    }
    return SIZE_MAX;
}

/**
 * @brief Splits a raw HTTP request string into 3 parts: request line, headers and body.  
 * 
 * @param raw Raw request line to parse.
 * @param raw_len length of the raw request string.
 * @param req_line Pointer to the string storing the request line (the first line of a request.) 
 * @param headers Pointer to the string storing the headers.
 * @param body Pointer to the string storing the body.
 * @return HttpParseStatus 
 */
HttpParseStatus split_request(sds raw, size_t raw_len, sds *req_line, sds *headers, sds *body)
{
    size_t idx = find_first(raw, raw_len, "\r\n\r\n", 4); /* First occurrence of `\r\n\r\n` separating headers from body */
    size_t line_end = find_first(raw, raw_len, "\r\n", 2); /* First occurrence of `\r\n` signifying the end of request line */
    
    if (idx == SIZE_MAX)
        return HTTP_PARSE_INCOMPLETE;

    *body = sdsnewlen(raw, raw_len);
    sdsrange(*body, idx + 4, raw_len);

    *req_line = sdsnewlen(raw, raw_len);
    sdsrange(*req_line, 0, line_end-1);

    *headers = sdsnewlen(raw, raw_len);
    sdsrange(*headers, line_end + 2, idx);

    return HTTP_PARSE_OK;
}

/**
 * @brief Turns a HTTP method string to a `HttpMethod` enum value.
 * 
 * @param token HTTP method string.
 * @return HttpMethod HTTP method Enumn.
 */
HttpMethod lookup_method(const sds token)
{
    for (unsigned long i = 0; i < sizeof(METHOD_TABLE)/sizeof(*METHOD_TABLE); i++) {
        if (strcmp(METHOD_TABLE[i].name, token) == 0){
            return METHOD_TABLE[i].method;
        }
    }
    return HTTP_METHOD_UNDEFINED;
}

/**
 * @brief Parses the first line of an HTTP request. Checks for HTTP version and catches malformed requests.
 * 
 * @param request_line Raw request line string to parse.
 * @param req Pointer to HttpRequest struct storing the parsed request.
 * @return HttpParseStatus 
 */
HttpParseStatus parse_request_line(sds request_line, HttpRequest *req)
{
    int token_count;
    sds *tokens = sdssplitlen(request_line, sdslen(request_line), " ", 1, &token_count);

    if (token_count != 3)
        return HTTP_PARSE_ERR_MALFORMED_REQUEST_LINE;

    if (sdscmp(tokens[2], sdsnew("HTTP/1.1")) != 0)
        return HTTP_PARSE_ERR_UNSUPPORTED_VERSION;

    if (strncmp(tokens[1], "/", 1) != 0)
        return HTTP_PARSE_ERR_MALFORMED_REQUEST_LINE;

    if (sdslen(tokens[1]) > MAX_URI_LEN)
        return HTTP_PARSE_ERR_URI_TOO_LONG;
    
    req->method = lookup_method(tokens[0]);
    req->path = tokens[1];
    req->version = tokens[2];

    return HTTP_PARSE_OK;
}

/**
 * @brief Parses the request's headers. Creates an array of `HttpHeader` structs storing key/value pairs.
 * 
 * @param headers Raw headers string to parse.
 * @param req Pointer to HttpRequest struct storing the parsed request.
 * @return HttpParseStatus 
 */
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
        sdstrim(header.name, " \r\n");

        header.value = sdsnew(lines[i]);
        sdsrange(header.value, colon_idx + 1, line_len);
        sdstrim(header.value, " \r\n");

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

/**
 * @brief HTTP request parsing pipeline and interface.
 * 
 * @param raw Raw HTTP request string.
 * @param len Length of the raw request HTTP string.
 * @param out Pointer to HttpRequest struct storing the parsed request.
 * @return HttpParseStatus 
 */
HttpParseStatus http_parse_request(const sds raw, size_t len, HttpRequest *out)
{
    sds request_line, headers, body;
    TRY(split_request(raw, len, &request_line, &headers, &body));
    
    TRY(parse_request_line(request_line, out));
    TRY(parse_headers(headers, out));
    parse_body(body, out);

    
    return HTTP_PARSE_OK;
}

/**
 * @brief Generate a HTTP response string from the provided `HttpResponse` struct.
 * 
 * @param resp Struct to generate the response string from. 
 * @param resp_buf Pointer to string storing the generated response string.
 * @param resp_len Pointer to variable storing the length of the generated response string.
 */
void http_build_response_str(HttpResponse resp, sds *resp_buf, size_t *resp_len)
{
    sds header_block = sdscatprintf(sdsempty(), "HTTP/1.1 %i %s\r\n", resp.status_code, resp.reason_phrase);
    for (size_t hc = 0; hc < resp.header_count; hc++)
        header_block = sdscatprintf(header_block, "%s: %s\r\n", resp.headers[hc].name, resp.headers[hc].value);
    header_block = sdscat(header_block, "\r\n");
    size_t header_len = sdslen(header_block);

    size_t total_len = header_len + resp.body_len;
    char *buf = malloc(total_len);

    memcpy(buf, header_block, header_len);
    memcpy(buf + header_len, resp.body, resp.body_len);

    *resp_buf = buf;
    *resp_len = total_len;

    sdsfree(header_block);
}

/**
 * @brief Get the mime type from the file extension.
 * 
 * @param filename 
 * @return sds mime type.
 */
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

/**
 * @brief Map HTTP response code to appropriate reason phrase.
 * 
 * @param status_code HTTP response code.
 * @return const sds Reason phrase.
 */
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

/**
 * @brief Add a "name: value" header to a `HttpResponse`
 * 
 * @param resp Pointer to the `HttpResponse` to add to.
 * @param name 
 * @param value 
 * @return true If successful.
 * @return false If unsuccessful.
 */
bool http_add_header(HttpResponse *resp, const sds name, const sds value)
{
    if (resp->header_count >= MAX_HEADERS) return false; 
    resp->headers[resp->header_count++] = (HttpHeader){ .name = name, .value = value };
    return true;
}