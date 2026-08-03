#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sds.h"
#include "network.h" 
#include "http.h"


void process_request(const char *raw, size_t len, char **response)
{
    sds request_str = sdsnew(raw);
    HttpRequest request;
    http_parse(request_str, len, &request);
}