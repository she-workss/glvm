#include "glvm/Network/UDP_ClientLinux.hpp"

namespace GLVM::core {
UDP_ClientLinux::UDP_ClientLinux(unsigned long port, const char* serverIP) :
    port(port),
    serverIP(serverIP) {
    /** UDP-socket creation
        @param AF_INET    protocol family (IPv4)
        @param SOCK_DGRAM socket type
        @param 0          protocol type. If we pass 0 then function will
    depending on socket type (For SOCK_DGRAM its UDP)
    **/
    if ((socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /// Socket addresss filling
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port =
        htons(port); ///< htons() function converts the unsigned short integer
                     ///< hostshort from host byte order to network byte order
    serverAddress.sin_addr.s_addr = inet_addr(
        serverIP
    ); ///< inet_addr() Convert Internet host address from numbers-and-dots
       ///< notation in CP into binary data in network byte order
}

char* UDP_ClientLinux::receive() {
    /// Receiving response from server
    int n = recvfrom(socketFileDescriptor, (char*)buffer, 1024, 0, NULL, NULL);
    buffer[n] = '\0'; ///< Complete string

    return buffer;
}

void UDP_ClientLinux::response() {
    /// Sending message to server
    sendto(
        socketFileDescriptor,
        (const char*)message,
        strlen(message),
        0,
        (const struct sockaddr*)&serverAddress,
        sizeof(serverAddress)
    );
}

UDP_ClientLinux::~UDP_ClientLinux() {
    /// Socket closing
    close(socketFileDescriptor);
}
}; // namespace GLVM::core
