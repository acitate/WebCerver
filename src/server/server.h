#ifndef __SERVER_H
#define __SERVER_H

#include "sds.h"
#include "../http/http.h"
#include "../resource/resource_resolver.h"

void server_process_request(const sds *req_str, size_t req_len, sds *resp_str, size_t *resp_len);
void build_success_response(HttpResponse *resp, int code, sds buf, size_t buf_len, sds path);
static int http_code_for_ParseStatus(HttpParseStatus pstatus);
static int http_code_for_PathStatus(PathStatus pstatus);
void build_error_response(HttpResponse *resp, int code);


#endif