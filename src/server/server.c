#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "sds.h"
#include "../http/http.h"
#include "../resource/resource_resolver.h"
#include "../resource/filesystem.h"
#include "server.h"

void server_process_request(const sds *req_str, size_t req_len, sds *resp_str, size_t *resp_len)
{
    HttpRequest request;
    HttpParseStatus hpstatus = http_parse_request(req_str, req_len, &request);
    HttpResponse response;

    if (hpstatus != HTTP_PARSE_OK) {
        build_error_response(&response, http_code_for_ParseStatus(hpstatus));
    } else if (request.method == HTTP_METHOD_UNDEFINED) {
        build_error_response(&response, 501);
    } else {
        sds buf;
        size_t buf_len;
        PathStatus pstatus = resolve_resource(request.path, sdslen(request.path), "root", &buf, &buf_len);
        if (pstatus != PATH_OK) {
            build_error_response(&response, http_code_for_PathStatus(pstatus));
        } else {
            build_success_response(&response, 200, buf, buf_len, request.path);
        }
    }

    http_build_response_str(response, resp_str, resp_len);
}


void build_success_response(HttpResponse *resp, int code, sds buf, size_t buf_len, sds path)
{
    http_add_header(resp, "Content-Type", get_mime_type(path));
    http_add_header(resp, "Content-Length", sdsfromlonglong(buf_len));
    http_add_header(resp, "Connection", "close");

    resp->body = buf;
    resp->body_len = buf_len;
    resp->status_code = code;
    resp->reason_phrase = http_reason_phrase(code);
}


void build_error_response(HttpResponse *resp, int code)
{
    http_add_header(resp, "Content-Type", get_mime_type(".html"));
    http_add_header(resp, "Connection", "close");
    
    resp->status_code = code;
    resp->reason_phrase = http_reason_phrase(code);
    resp->body = sdscatfmt(sdsempty(), "<!DOCTYPE HTML>\n<html lang='en'>\n    <head>\n        <meta charset='utf-8'>\n        <style type='text/css'>\n            :root {\n                color-scheme: light dark;\n            }\n        </style>\n        <title>Error response</title>\n    </head>\n    <body>\n        <h1>Error response</h1>\n        <p>Error code: %i</p>\n        <p>Message: %s.</p>\n    </body>\n</html>", resp->status_code, resp->reason_phrase);
    http_add_header(resp, "Content-Length", "0");
    resp->headers[2].value = sdscatfmt(sdsempty(), "%i", sdslen(resp->body));
}


static int http_code_for_ParseStatus(HttpParseStatus pstatus) 
{
    switch (pstatus) {
        case HTTP_PARSE_ERR_MALFORMED_REQUEST_LINE: return 400;
        case HTTP_PARSE_ERR_URI_TOO_LONG: return 414;
        case HTTP_PARSE_ERR_UNSUPPORTED_VERSION: return 505;
        case HTTP_PARSE_ERR_MALFORMED_HEADER: return 400;
        case HTTP_PARSE_ERR_HEADER_TOO_LONG: return 431;
        case HTTP_PARSE_ERR_TOO_MANY_HEADERS: return 431;
        case HTTP_PARSE_ERR_BUFFER_OVERRUN: return 500;
        case HTTP_PARSE_OK:
        case HTTP_PARSE_INCOMPLETE:
        default: return 500;
    }
}


static int http_code_for_PathStatus(PathStatus pstatus) 
{
    switch (pstatus)
    {
    case PATH_ERR_MALFORMED_ENCODING: 
    case PATH_ERR_NULL_BYTE:
    case PATH_ERR_CONTROL_CHAR: return 400;
    case PATH_ERR_TOO_LONG: return 414;
    case PATH_ERR_NOT_FOUND: return 404;
    case PATH_ERR_HIDDEN_PATH: return 404;
    case PATH_ERR_FORBIDDEN: return 404; // Not 403 in order to keep server data hidden
    case PATH_ERR_IS_DIRECTORY: return 403;
    case PATH_ERR_UNEXPECTED: return 500;
    case PATH_OK:
    default: return 500;
    }
}