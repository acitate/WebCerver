#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sds.h"
#include "../http/http.h"
#include "../resource/resource_resolver.h"
#include "../resource/filesystem.h"


void server_process_request(const sds *req_str, size_t req_len, sds *resp_str, size_t *resp_len)
{
    HttpRequest request;
    http_parse_request(req_str, req_len, &request);
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