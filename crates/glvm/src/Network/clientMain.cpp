#include "glvm/UDP_ClientLinux.hpp"

int main() {
    GLVM::core::UDP_ClientLinux clientLinux;
    clientLinux.response();
    char* massageFromServer = clientLinux.receive();
    printf("Message from server: %s\n", massageFromServer);

    return 0;
}
