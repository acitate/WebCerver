#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


// Creates a TCP server socket bound to given port on all interfaces
// Returns file descriptor on success, exits on failure
int get_server_socket(int port) 
{
    // Create socket: IPv4, TCP
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Allow address reuse to avoid "Address already in use" on restart
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind to all interfaces on given port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening with backlog of 3 pending connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}


// Converts sockaddr_in to string format "[IP]:[PORT]"
// Caller must free returned string
char *get_address_string(struct sockaddr_in *addr_in) {
    // struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN);
    int port = ntohs(addr_in->sin_port);
    
    char *result = malloc(strlen(ip_str) + 1 + 5 + 2 + 1); // IP + : + port + [] + null
    if (!result) return NULL;
    sprintf(result, "%s:%d", ip_str, port);
    return result;
}