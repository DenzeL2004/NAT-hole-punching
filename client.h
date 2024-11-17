#pragma once

#include "utils.h"

typedef struct Client_ {
    int sock;
    ClientAddresses addresses;
    struct sockaddr_in local_addr;

    size_t id;
} Client;

int CtorClient(Client* self, const Address const* addr);

size_t ClientRegistrate(Client* client, struct sockaddr_in* server_sock_addr);

struct sockaddr_in DefineConnection(Client* client, ClientAddresses* anoter_addr);

void ClientCommunicate(Client* client, struct sockaddr_in* another_client_addr);

int DtorClient(Client* self);