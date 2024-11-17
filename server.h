#pragma once 

#include "utils.h"

typedef struct Server_ {
    int sock;
    struct sockaddr_in addr;

    size_t cnt_active_clt;
    ClientAddresses clients[MAX_COUNT_CLIENTS];
} Server;

int CtorServer(Server* self, const Address const* addr);

int DtorServer(Server* self);

ClientAddresses* CurFreeSlot(Server *server);

void ServerAcceptClient(Server *server);