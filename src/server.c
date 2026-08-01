#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "network.h" 
#define PORT 8080
#define BUFFER_SIZE 65536


int main()
{
    int server_fd, client_fd;
    struct sockaddr_in client_addr;
    server_fd = get_server_socket(PORT);
    
    while (1) 
    {
        socklen_t client_addrlen = sizeof(client_addr);

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen);
        if (client_fd == -1) {
            perror("[\x1b[31mServer\x1b[0m] accept failed");
            continue;
        }

        char *client_addr_str = get_address_string(&client_addr);
        printf("[\x1b[32mServer\x1b[0m] receivced connection from %s\n", client_addr_str);
        free(client_addr_str);
        close(client_fd);
    }
}