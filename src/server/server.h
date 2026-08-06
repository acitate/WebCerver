#ifndef __SERVER_H
#define __SERVER_H

void process_request(const char *raw, size_t len, char **response);

#endif