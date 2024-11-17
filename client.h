#pragma once

#include "utils.h"

typedef struct Client_ {
    int sock;
    ClientAddresses addresses;
    struct sockaddr_in local_addr;
} Client;

int CtorClient(Client* self, const Address const* addr);