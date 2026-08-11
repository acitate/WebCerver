#ifndef __HTTP_H
#define __HTTP_H


typedef enum 
{
    HTTP_METHOD_GET, 
    HTTP_METHOD_UNDEFINED
} HttpMethod;


typedef struct
{
    sds name;
    sds value;
} HttpHeader;


typedef struct 
{
    HttpMethod method;
    sds path;
    sds version;
    HttpHeader headers[32];
    size_t header_count;
    sds body;
    size_t body_len;
} HttpRequest;


typedef struct 
{
    int status_code;
    sds reason_phrase;
    HttpHeader headers[16];
    size_t header_count;
    sds body;
    size_t body_len;
} HttpResponse;


typedef enum 
{
    HTTP_PARSE_OK = 0,
    HTTP_PARSE_ERR_MALFORMED_REQUEST_LINE,
    HTTP_PARSE_ERR_URI_TOO_LONG,
    HTTP_PARSE_ERR_UNSUPPORTED_VERSION,
    HTTP_PARSE_ERR_MALFORMED_HEADER,
    HTTP_PARSE_ERR_HEADER_TOO_LONG,
    HTTP_PARSE_ERR_TOO_MANY_HEADERS,
    HTTP_PARSE_INCOMPLETE,
    HTTP_PARSE_ERR_BUFFER_OVERRUN
} HttpParseStatus;


void http_parse_request(const sds raw, size_t len, HttpRequest *out);
void http_build_response(HttpResponse resp, sds *resp_buf, size_t *resp_len);
void http_response_404(sds *resp_buf, size_t *resp_len);

#endif