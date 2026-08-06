#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sds.h"
#include "../http/http.h"


void process_request(const char *raw, size_t len, char **response)
{
    sds request_str = sdsnew(raw);
    HttpRequest request;
    http_parse_request(request_str, len, &request);
}