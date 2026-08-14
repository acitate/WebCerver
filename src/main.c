#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "net/network.h"
#include "server/server.h"
#define PORT 8080
#define BUFFER_SIZE 65536


static void *handle_connection(void *arg)
{   
    int client_fd = (int)(intptr_t) arg;
    char buffer[BUFFER_SIZE];
    size_t buf_len = network_read_bytes(client_fd, buffer, sizeof(buffer));

    char *resp;
    size_t resp_len;
    server_process_request(buffer, buf_len, &resp, &resp_len);

    network_close(client_fd);
}


int main()
{
    int server_fd = get_server_socket(8080);
    while (1) {
        int client_fd = network_accept(server_fd);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_connection, (void *)(intptr_t) client_fd);
    }
}