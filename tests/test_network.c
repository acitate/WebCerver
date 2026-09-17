#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/net/network.h"

static void close_checked(int descriptor)
{
    int result = network_close(descriptor);
    assert(result == 0);
}

static void test_read_and_eof(void)
{
    int sockets[2];
    int result = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
    assert(result == 0);

    const char payload[] = {'a', '\0', 'b', (char)0xff, 'c'};
    size_t sent = 0;
    while (sent < sizeof(payload)) {
        ssize_t count = send(sockets[0], payload + sent, sizeof(payload) - sent, 0);
        assert(count > 0 && (size_t)count <= sizeof(payload) - sent);
        sent += (size_t)count;
    }
    result = shutdown(sockets[0], SHUT_WR);
    assert(result == 0);

    size_t received = 0;
    while (received < sizeof(payload)) {
        unsigned char guarded[] = {0xa5, 0xa5, 0xa5, 0xa5};
        ssize_t count = network_read_bytes(sockets[1], (char *)guarded + 1, 2);
        assert(count > 0 && count <= 2);
        assert((size_t)count <= sizeof(payload) - received);
        assert(guarded[0] == 0xa5 && guarded[3] == 0xa5);
        assert(guarded[1 + count] == 0xa5);
        assert(memcmp(guarded + 1, payload + received, (size_t)count) == 0);
        received += (size_t)count;
    }

    char buffer;
    ssize_t count = network_read_bytes(sockets[1], &buffer, sizeof(buffer));
    assert(count == 0);
    close_checked(sockets[0]);
    close_checked(sockets[1]);
}

static void test_send(void)
{
    int sockets[2];
    int result = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
    assert(result == 0);

    char payload[] = {'x', '\0', (char)0xff, 'y'};
    ssize_t count = network_send_bytes(sockets[0], payload, 0);
    assert(count == 0);

    size_t sent = 0;
    while (sent < sizeof(payload)) {
        count = network_send_bytes(sockets[0], payload + sent, sizeof(payload) - sent);
        assert(count > 0 && (size_t)count <= sizeof(payload) - sent);
        sent += (size_t)count;
    }

    char buffer[sizeof(payload)];
    size_t received = 0;
    while (received < sizeof(buffer)) {
        count = recv(sockets[1], buffer + received, sizeof(buffer) - received, 0);
        assert(count > 0 && (size_t)count <= sizeof(buffer) - received);
        received += (size_t)count;
    }
    assert(memcmp(buffer, payload, sizeof(payload)) == 0);
    close_checked(sockets[0]);
    close_checked(sockets[1]);
}

static void test_invalid_descriptors(void)
{
    char buffer = 'x';
    errno = 0;
    ssize_t count = network_read_bytes(-1, &buffer, sizeof(buffer));
    assert(count == -1 && errno == EBADF);

    errno = 0;
    count = network_send_bytes(-1, &buffer, sizeof(buffer));
    assert(count == -1 && errno == EBADF);

    errno = 0;
    int result = network_close(-1);
    assert(result == -1 && errno == EBADF);

    result = network_accept(-1);
    assert(result == -1);
}

static void test_close(void)
{
    int descriptor = socket(AF_INET, SOCK_STREAM, 0);
    assert(descriptor >= 0);
    close_checked(descriptor);

    errno = 0;
    int result = fcntl(descriptor, F_GETFD);
    assert(result == -1 && errno == EBADF);
}

static void test_listener_and_accept(void)
{
    int listener = get_server_socket(0);
    assert(listener >= 0);

    struct sockaddr_in address = {0};
    socklen_t address_len = sizeof(address);
    int result = getsockname(listener, (struct sockaddr *)&address, &address_len);
    assert(result == 0);
    assert(address.sin_family == AF_INET);
    assert(address.sin_port != 0);
    assert(address.sin_addr.s_addr == htonl(INADDR_ANY));

    int reuse = 0;
    socklen_t reuse_len = sizeof(reuse);
    result = getsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, &reuse_len);
    assert(result == 0 && reuse != 0);

    int client = socket(AF_INET, SOCK_STREAM, 0);
    assert(client >= 0);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    result = connect(client, (struct sockaddr *)&address, sizeof(address));
    assert(result == 0);

    int accepted = network_accept(listener);
    assert(accepted >= 0);
    char payload = 'x';
    ssize_t count = send(client, &payload, sizeof(payload), 0);
    assert(count == 1);
    char buffer = 0;
    count = network_read_bytes(accepted, &buffer, sizeof(buffer));
    assert(count == 1 && buffer == payload);

    close_checked(accepted);
    close_checked(client);
    close_checked(listener);
}

static void test_bind_failure(void)
{
    int listener = get_server_socket(0);
    assert(listener >= 0);
    struct sockaddr_in address = {0};
    socklen_t address_len = sizeof(address);
    int result = getsockname(listener, (struct sockaddr *)&address, &address_len);
    assert(result == 0);

    fflush(NULL);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        alarm(10);
        close_checked(listener);
        int unexpected = get_server_socket(ntohs(address.sin_port));
        close_checked(unexpected);
        _exit(EXIT_SUCCESS);
    }

    int status = 0;
    pid_t waited = waitpid(child, &status, 0);
    assert(waited == child);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == EXIT_FAILURE);
    close_checked(listener);
}

int main(void)
{
    alarm(10);
    test_read_and_eof();
    test_send();
    test_invalid_descriptors();
    test_close();
    test_listener_and_accept();
    test_bind_failure();
    alarm(0);
    printf("All network tests passed.\n");
    return 0;
}
