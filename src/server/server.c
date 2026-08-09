#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sds.h"
#include "../http/http.h"
#include "../resource/resource_resolver.h"
#include "../resource/filesystem.h"


void server_process_request(const char *raw, size_t len, char **response, size_t *resp_len)
{
    sds request_str = sdsnew(raw);
    HttpRequest request;
    http_parse_request(request_str, len, &request);
}