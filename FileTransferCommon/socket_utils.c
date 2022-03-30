#include "socket_utils.h"

boolean socket_initialize(WSADATA* wsaData) {
    return WSAStartup(MAKEWORD(2, 2), wsaData);
}

int socket_listen(SOCKET* sock, SOCKADDR_IN* sock_addr, const uint16_t port) {
    int status;
    SOCKADDR_IN sockaddr;
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if ((*sock) == -1) {
        return STATUS_ERR_SOCK_CREATE;
    }


    sockaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    sockaddr.sin_port = htons(port);
    sockaddr.sin_family = AF_INET;
    status = bind(*sock, (SOCKADDR_IN*)&sockaddr, sizeof(sockaddr));
    if (status == -1) {
        return STATUS_ERR_SOCK_BIND;
    }

    if ((listen(*sock, SOCKET_BACKLOG)) < 0) {
        return STATUS_ERR_SOCK_LISTEN;
    }

    return STATUS_SUCCESS;
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

void safe_recv(SOCKET* sock, char* buf, int len) {
    char hold[1];
    int left = len;
    int idx;
    int status;

    for (idx = 0; idx < len; idx++) buf[idx] = 0;

    while (left > 0) {
        idx = len - left;
        hold[0] = 0;
        status = recv(sock, hold, 1, 0);
        if (status != -1) {
            buf[idx] = hold[0];
            left -= 1;
        }
    }
}

void safe_send(SOCKET* sock, char* buf, int len) {
    char* hold;
    int status;
    int sent = 0;

    do {
        hold = malloc(sizeof(char));
    } while (hold == NULL);

    while (sent < len) {
        hold[0] = buf[sent];
        status = send(sock, hold, 1, 0);
        if (status != -1) {
            sent++;
        }
    }
}