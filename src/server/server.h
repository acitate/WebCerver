#ifndef __SERVER_H
#define __SERVER_H

void server_process_request(const char *raw, size_t len, char **response, size_t *resp_len);

#endif