#include "glvm/Network/UDP_ServerLinux.hpp"

namespace GLVM::core {
UDP_ServerLinux::UDP_ServerLinux(unsigned long port) : port(port) {
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
    serverAddress.sin_addr.s_addr =
        INADDR_ANY; ///< Receive messages from any addresses
    serverAddress.sin_port =
        htons(port); ///< htons() function converts the unsigned short integer
                     ///< hostshort from host byte order to network byte order

    /** Socket to port binding
        ip   - specific machine address
        port - specific application intrance
    **/
    if (bind(
            socketFileDescriptor,
            (const struct sockaddr*)&serverAddress,
            sizeof(serverAddress)
        )
        < 0) {
        perror("Bind failed");
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }
}

char* UDP_ServerLinux::receive() {
    int n = recvfrom(
        socketFileDescriptor,
        (char*)buffer,
        maxBufferSize,
        0,
        (struct sockaddr*)&clientAddress,
        &clientAddressLength
    );
    if (n < 0) {
        perror("Receive failed");
    }

    buffer[n] = '\0'; ///< Complete string
    return buffer;
}

void UDP_ServerLinux::response() {
    // Sending response to client
    const char* response = "Message received!";
    sendto(
        socketFileDescriptor,
        (const char*)response,
        strlen(response),
        0,
        (const struct sockaddr*)&clientAddress,
        clientAddressLength
    );
}

UDP_ServerLinux::~UDP_ServerLinux() {
    /// Socket closing
    close(socketFileDescriptor);
}
}; // namespace GLVM::core
