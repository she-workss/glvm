#ifndef UDP_CLIENT_LINUX
#define UDP_CLIENT_LINUX

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace GLVM::core {
struct UDP_ClientLinux {
    unsigned long port;
    const char* serverIP;
    static const unsigned int maxBufferSize = 1024;
    int socketFileDescriptor;
    sockaddr_in serverAddress;
    const char* message = "Hello, UDP server!";
    char buffer[maxBufferSize];

    UDP_ClientLinux(
        unsigned long port = 8080,
        const char* serverIP = "127.0.0.1"
    );
    char* receive();
    void response();
    ~UDP_ClientLinux();
};
}; // namespace GLVM::core

#endif
