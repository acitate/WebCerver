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
    char name[64];
    char value[256];
} HttpHeader;


typedef struct 
{
    HttpMethod method;
    char path[1024];
    char versrion[16];
    HttpHeader headers[32];
    size_t header_count;
    char *body;
    size_t body_len;
} HttpRequest;

void http_parse(const sds raw, size_t len, HttpRequest *out);

#endif