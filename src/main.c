#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
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
    free(args->webroot);
    free(args);

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
            struct connection_args *args = malloc(sizeof(*args));

            if (args == NULL) {
                return 1;
            }

            args->webroot = malloc(sizeof(server_conf.webroot));

            if (args->webroot == NULL) {
                free(args);
                return 1;
            }

            args->client_fd = client_fd;
            strcpy(args->webroot, server_conf.webroot);

            pthread_t tid;
            pthread_create(&tid, NULL, handle_connection, args);
            pthread_detach(tid);
        }
    }
    
    return 0;
}