// FileTransferSender.c : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "stdio.h"
#include "winsock2.h"

boolean socket_initialize(WSADATA* wsaData) {
    return WSAStartup(MAKEWORD(2, 2), wsaData);
}

int socket_connect(SOCKET* sock, const char* dest, const u_short port) {
    SOCKADDR_IN sockaddr;
    int status;
    sockaddr.sin_addr.s_addr = inet_addr(dest);
    sockaddr.sin_port = htons(port);
    sockaddr.sin_family = AF_INET;
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    status = connect(*sock, &sockaddr, sizeof(sockaddr));
    return status;
}

int main(const int argc, const char *argv[])
{
    char *remote_addr, *remote_port;
    SOCKET sock;
    WSADATA wsaData;
    int status;

    if (socket_initialize(&wsaData) != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return 1;
    }

    status = socket_connect(&sock, "127.0.0.1", 3490);
    printf("hi, status is %d\n", status);
    printf("argc is %d\n", argc);

    return 0;
}