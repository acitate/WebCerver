#ifndef __NETWORK_H
#define __NETWORK_H

int get_server_socket(int port);
int network_accept(int sock_fd);
size_t network_read_bytes(int sock_fd,char *out, size_t out_len);
int network_close(int sock_fd);
size_t network_send_bytes(int sock_fd, char *out, size_t out_len);

#endif