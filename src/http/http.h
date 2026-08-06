#include <stdio.h>

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


void http_parse_request(const sds raw, size_t len, HttpRequest *out);

#endif