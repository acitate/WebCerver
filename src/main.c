#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "net/network.h"
#include "server/server.h"
#include "cli/cli.h"

#define BUFFER_SIZE 65536


struct connection_args {
    int client_fd;
    char *webroot;
};


static void *handle_connection(void *arg)
{   
    struct connection_args *args = arg;

    char buffer[BUFFER_SIZE];
    size_t buf_len = network_read_bytes(args->client_fd, buffer, sizeof(buffer));

    char *resp;
    size_t resp_len;
    server_process_request(buffer, buf_len, &resp, &resp_len, args->webroot);

    network_send_bytes(args->client_fd, resp, resp_len);

    network_close(args->client_fd);

    return NULL;
}


int main(int argc, char **argv)
{
    ServerConf server_conf;
    CliResult cli_result = cli_parse(argc, argv, &server_conf);
    
    if (!cli_result) {
        int server_fd = get_server_socket(server_conf.port);

        while (1) {
            int client_fd = network_accept(server_fd);

            struct connection_args args = {
                .client_fd = client_fd,
                .webroot = server_conf.webroot
            };

            pthread_t tid;
            pthread_create(&tid, NULL, handle_connection, &args);
        }
    }
    
    return 0;
}