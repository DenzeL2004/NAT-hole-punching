#include "server.h"

int CtorServer(Server* self, const Address const* addr) {
    assert(self != NULL && "self is null pointer");
    assert(addr != NULL && "self is null pointer");
    
    self->sock = socket(AF_INET, SOCK_DGRAM, 0);
    self->addr = GetSockAddr(addr);

    if (bind(self->sock, (struct sockaddr*)&self->addr, sizeof(self->addr))) {
        fprintf(stderr, "Something went wrong with bind server by ");
        PrintAddress(stderr, addr);
        perror("\n");
        return FAILED_BIND_SOCKET;
    }

    self->cnt_active_clt = 0;

    return 0;
}

int DtorServer(Server* self) {
    assert(self != NULL && "self is null pointer");

    if (close(self->sock)) {
        fprintf(stderr, "Failed close Server's socket = %d", self->sock);
        return FAILED_CLOSE_SOCKET;
    }    

    return 0;
}

ClientAddresses* CurFreeSlot(Server *server) {
    assert(server != NULL && "self is null pointer");

    return server->clients + server->cnt_active_clt;
}

void ServerAcceptClient(Server *server) {
    assert(server != NULL && "self is null pointer");

    ClientAddresses* client_addr = CurFreeSlot(server);

    struct sockaddr_in client_sock_addr;
    socklen_t len = sizeof(struct sockaddr_in);

    Packet request = {};
    int res = recvfrom(server->sock, (void*)&request, sizeof(Packet), MSG_WAITALL,
                                        (struct sockaddr*)&client_sock_addr, &len);

    if (res > 0 && request.type != REG_CLIENT) {
        request.type = NONE;
        sendto(server->sock, (void*)&request, sizeof(Packet), 0,
                        (struct sockaddr*)&client_sock_addr, len);
        return;
    }


    *client_addr = request.info.addresses;

    client_addr->global.ip   = client_sock_addr.sin_addr.s_addr;
    client_addr->global.port = client_sock_addr.sin_port;

    printf("\nAccept client(%lu).\n", server->cnt_active_clt);
    printf("Local address: "), PrintAddress(stdout, &client_addr->local);
    printf("\nGlobal address: "), PrintAddress(stdout, &client_addr->global);

    Packet answer = {RET_ID, server->cnt_active_clt};
    server->cnt_active_clt++;
   
    sendto(server->sock, (void*)&answer, sizeof(Packet), 0,(struct sockaddr*)&client_sock_addr, len);   
}

int ServerCommunicate(Server *server) {
    assert(server != NULL && "self is null pointer");

    struct sockaddr_in client_sock_addr;
    while(1) {
        socklen_t len = sizeof(struct sockaddr_in);

        Packet request = {};
        int res = recvfrom(server->sock, (void*)&request, sizeof(request), MSG_WAITALL,
                        (struct sockaddr*)&client_sock_addr, &len);
        
        if (res > 0 && request.type != GET_CLIENT_ADDRESS) {
            request.type = NONE;
            sendto(server->sock, (void*)&request, sizeof(Packet), 0,
                        (struct sockaddr*)&client_sock_addr, len);
            continue;
        }

        size_t other = (request.info.id + 1) % MAX_COUNT_CLIENTS;

        Packet answer = {RET_CLIENT_ADDRESS, server->clients[other]};
        sendto(server->sock, (void*)&answer, sizeof(Packet), 0,
                (struct sockaddr*)&client_sock_addr, len);
    }
}

void NATPunching(Server* server) {
    assert(server != NULL && "self is null pointer");

    while (server->cnt_active_clt < MAX_COUNT_CLIENTS) {
        ServerAcceptClient(server);
    }

   ServerCommunicate(server);
}

int main(int argc, char* argv[]) {
    Address addr;
    ParseArguments(2, argv + 1, &addr);
    Server server;
    CtorServer(&server, &addr);
    
    NATPunching(&server);

    DtorServer(&server);
}