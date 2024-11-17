#include "client.h"

int CtorClient(Client* self, const Address const* addr) { 
    assert(self != NULL && "self is null pointer");
    assert(addr != NULL && "self is null pointer");
    
    self->sock = socket(AF_INET, SOCK_DGRAM, 0);
    self->addresses.local = *addr;
    self->local_addr = GetSockAddr(addr);

    if (bind(self->sock, (struct sockaddr*)&self->local_addr, sizeof(self->local_addr))) {
        fprintf(stderr, "Something went wrong with bind client by ");
        PrintAddress(stderr, addr);
        perror("\n");
        return FAILED_BIND_SOCKET;
    }

    return 0;
}

int DtorClient(Client* self) { 
    assert(self != NULL && "self is null pointer");

    if (close(self->sock)) {
        fprintf(stderr, "Failed close client's socket = %d", self->sock);
        return FAILED_CLOSE_SOCKET;
    }    

    return 0;
}

size_t ClientRegistrate(Client* client, struct sockaddr_in* server_sock_addr) { 
    assert(client != NULL && "client is null pointer");
    assert(server_sock_addr != NULL && "server_sock_addr is null pointer");

    socklen_t len = sizeof(struct sockaddr_in);    
    Packet request = {REG_CLIENT, client->addresses};
    sendto(client->sock, (void*)&request, sizeof(Packet), 0,
                    (struct sockaddr*)server_sock_addr, len);

    Packet answer = {};
    recvfrom(client->sock, (void*)&answer, sizeof(Packet), 0,
                        (struct sockaddr*)server_sock_addr, &len);
    
    if (answer.type != RET_ID) {
        fprintf(stderr, "Failed get idfor client from server.\n");
        return FAILED_GET_ID;
    }

    client->id = answer.info.id;
}   

ClientAddresses ClientGetAddressesAnoter(Client* client, struct sockaddr_in* server_sock_addr) { 
    assert(client != NULL && "client is null pointer");
    assert(server_sock_addr != NULL && "server_sock_addr is null pointer");

    ClientAddresses another_address = {};
    socklen_t len = sizeof(struct sockaddr_in);   
    while (1) {
        Packet request = {GET_CLIENT_ADDRESS, client->id};
        sendto(client->sock, (void*)&request, sizeof(Packet), 0,
                (struct sockaddr*)server_sock_addr, len);
            
        Packet answer = {};
        recvfrom(client->sock, (void*)&answer, sizeof(Packet), MSG_WAITALL,
                (struct sockaddr*)server_sock_addr, &len);

        if (answer.type != RET_CLIENT_ADDRESS) {
            continue;
        }

        another_address = answer.info.addresses;
        break;
    }

    return another_address;
} 

struct sockaddr_in DefineConnection(Client* client, ClientAddresses* anoter_addr) {
    assert(client != NULL && "client is null pointer");
    assert(anoter_addr != NULL && "anoter_addr is null pointer");

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(client->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in another_sock_addr = {};
    struct sockaddr_in another_local_addr   = GetSockAddr(&anoter_addr->local);
    struct sockaddr_in another_global_addr  = GetSockAddr(&anoter_addr->global);

    socklen_t len = sizeof(struct sockaddr_in);
    size_t flag = 0;

    while (1) {
        flag = 1;
        sendto(client->sock, (void*)&flag, sizeof(flag), 0,
                (struct sockaddr*)&another_global_addr, len);
        
        size_t size = recvfrom(client->sock, (void*)&flag, sizeof(flag), MSG_WAITALL,
                        (struct sockaddr*)&another_sock_addr, &len);
        if (size > 0 && flag == 1) {
            printf("Global connection.\n");
            break;
        }

        if (another_local_addr.sin_addr.s_addr == client->addresses.local.ip) {
            continue;
        }

        flag = 2;
        sendto(client->sock, (void*)&flag, sizeof(flag), 0,
                (struct sockaddr*)&another_local_addr, len);

        size = recvfrom(client->sock, (void*)&flag, sizeof(flag), MSG_WAITALL,
                        (struct sockaddr*)&another_sock_addr, &len);

        if (size > 0 && flag == 2) {
            printf("Local connection.\n");
            break;
        }
    }

    sendto(client->sock, (void*)&flag, sizeof(flag), 0,
                    (struct sockaddr*)&another_sock_addr, len);

    return another_sock_addr;
}

void ClientCommunicate(Client* client, struct sockaddr_in* another_client_addr) {
    assert(client != NULL && "client is null pointer");
    assert(another_client_addr != NULL && "another_client_addr is null pointer");

    struct sockaddr_in plug_sock;
    socklen_t len = sizeof(struct sockaddr_in);
    size_t num = 1;

    while (num > 0) {
        if (client->id % 2 == 0) {
            printf("Send: %lu\n", num, );
            sendto(client->sock, (void*)&num, sizeof(num), 0,
                            (struct sockaddr*)another_client_addr, len);

            recvfrom(client->sock, (void*)&num, sizeof(num), 0,
                            (struct sockaddr*)&plug_sock, &len);

            printf("Get: %lu\n", num); 
        } else {
            recvfrom(client->sock, (void*)&num, sizeof(num), 0,
                            (struct sockaddr*)&plug_sock, &len);
    
            printf("Get: %lu\n", num);

            num <<= 1;
            printf("Send: %lu\n", num);
            
            sendto(client->sock, (void*)&num, sizeof(num), 0,
                        (struct sockaddr*)another_client_addr, len);
        }
    }
}

int main(int argc, char* argv[]) { 
    Address client_address = {};
    ParseArguments(2, argv + 1, &client_address);
    // PrintAddress(stdout, &client_address);
    // printf("\n");

    Address server_address = {};
    ParseArguments(2, argv + 3, &server_address);
    // PrintAddress(stdout, &server_address);
    // printf("\n");

    Client client = {};
    CtorClient(&client, &client_address);
    struct sockaddr_in server_sock_addr = GetSockAddr(&server_address);

    printf("Try connect with server.\n");
    ClientRegistrate(&client, &server_sock_addr);
    printf("Connection success. Client id: %lu.\n", client.id);

    ClientAddresses another_addres = ClientGetAddressesAnoter(&client, &server_sock_addr);
    printf("Another address: "), PrintAddress(stdout, &another_addres.local);
    printf("\n");

    struct sockaddr_in another_sock_addr = DefineConnection(&client, &another_addres);
    ClientCommunicate(&client, &another_sock_addr);

    DtorClient(&client);
}