#include "socket_utils.h"

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