// #ifdef NETWORK_H
// #define NETWORK_H

int get_server_socket(int port);
char *get_address_string(struct sockaddr_in *addr_in);

// #endif