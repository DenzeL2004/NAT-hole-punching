#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

typedef enum Errors_ {
    INVALID_ARGUMENTS   = -1,
    INVALID_ADDR        = -2,
    INVALID_PORT        = -3,
    FAILED_BIND_SOCKET  = -4,
    FAILED_CLOSE_SOCKET = -5,
    FAILED_GET_ID       = -6,
} Errors;

#define MAX_COUNT_CLIENTS 2

typedef struct Address_ {
    in_addr_t ip;
    in_port_t port;

} Address;

typedef struct ClientAddresses_ {
    Address local;
    Address global;
} ClientAddresses;

typedef enum PacketType_ {
    NONE,
    REG_CLIENT,
    RET_ID,
    RET_CLIENT_ADDRESS,
    GET_CLIENT_ADDRESS
} PacketType;

typedef struct Packet_ {
    PacketType type;
    union {
        ClientAddresses addresses;
        size_t id;
    } info;
} Packet;

Errors ParseArguments(int argc, char* argv[], Address* addr);

struct sockaddr_in GetSockAddr(const Address const* addr);

void PrintAddress(FILE *stream,  const Address const* addr);
