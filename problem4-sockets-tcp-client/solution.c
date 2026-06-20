#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int send_all(int fd, const unsigned char *buffer, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = send(fd, buffer + sent, size - sent, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

static int recv_all(int fd, unsigned char *buffer, size_t size) {
    size_t received = 0;
    while (received < size) {
        ssize_t n = recv(fd, buffer + received, size - received, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return 0;
        }
        received += (size_t)n;
    }
    return 1;
}

static void int32_to_le(int32_t value, unsigned char bytes[4]) {
    uint32_t u = (uint32_t)value;
    bytes[0] = (unsigned char)(u & 0xffu);
    bytes[1] = (unsigned char)((u >> 8) & 0xffu);
    bytes[2] = (unsigned char)((u >> 16) & 0xffu);
    bytes[3] = (unsigned char)((u >> 24) & 0xffu);
}

static int32_t le_to_int32(const unsigned char bytes[4]) {
    uint32_t u = ((uint32_t)bytes[0]) |
                 ((uint32_t)bytes[1] << 8) |
                 ((uint32_t)bytes[2] << 16) |
                 ((uint32_t)bytes[3] << 24);
    return (int32_t)u;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ipv4_addr> <port>\n", argv[0]);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    char *end = NULL;
    long port = strtol(argv[2], &end, 10);
    if (*argv[2] == '\0' || *end != '\0' || port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address\n");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    int32_t value;
    while (scanf("%" SCNd32, &value) == 1) {
        unsigned char request[4];
        unsigned char response[4];
        int32_to_le(value, request);

        if (send_all(sock, request, sizeof(request)) < 0) {
            close(sock);
            return 0;
        }

        int received = recv_all(sock, response, sizeof(response));
        if (received == 0) {
            close(sock);
            return 0;
        }
        if (received < 0) {
            perror("recv");
            close(sock);
            return 1;
        }

        printf("%" PRId32 "\n", le_to_int32(response));
    }

    close(sock);
    return 0;
}
