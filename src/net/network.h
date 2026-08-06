#ifndef __NETWORK_H
#define __NETWORK_H

int get_server_socket(int port);
int network_accept(int sock_fd);
size_t read_request(int sock_fd,char *out, size_t out_len);
int network_close(int sock_fd);

#endif