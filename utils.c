#include "utils.h"

Errors ParseArguments(int argc, char* argv[], Address* addr) {
    assert(argv != NULL && "argv is null pointer");
    assert(addr != NULL && "addr is null pointer");

    if (argc != 2) {
        fprintf(stderr, "Invalid count arguments %u, must be 2.\n", argc);
        return INVALID_ARGUMENTS;
    }

    int err = inet_pton(AF_INET, argv[0], &addr->ip);
    if (err <= 0) {
        fprintf(stderr, "Invalid address = %s.\n", argv[0]);
        return INVALID_ADDR;
    }

    addr->port = htons(atoi(argv[1]));
    if (addr->port == 0) {
        printf("Invalid port = %s.\n", argv[1]);
        return INVALID_PORT;
    }

    return 0;
}

struct sockaddr_in GetSockAddr(const Address const* addr) {
    assert(addr != NULL && "addr is null pointer");
    
    struct sockaddr_in sock_addr = {};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = addr->port;
    sock_addr.sin_addr.s_addr = addr->ip;

    return sock_addr;
}

void PrintAddress(FILE *stream,  const Address const* addr) {
    assert(addr != NULL && "addr is null pointer");

    char str[INET_ADDRSTRLEN] = "";
    inet_ntop(AF_INET, &addr->ip, str, sizeof(str));
    int port = ntohs(addr->port);
    fprintf(stream, "%s:%hu", str, port);
}
