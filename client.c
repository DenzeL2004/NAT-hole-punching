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

int main(int argc, char* argv[]) { 
    Address client_address = {};
    ParseArguments(2, argv + 1, &client_address);

    Address server_address = {};
    ParseArguments(2, argv + 3, &server_address);

    Client client = {};
    CtorClient(&client, &client_address);

    

}