#include "socket_utils.h"
#define PORT_SENDER "6342"
#define PORT_RECEIVER "6343"
boolean socket_initialize(WSADATA* wsaData) {
    return WSAStartup(MAKEWORD(2, 2), wsaData);
}

int socket_listen(SOCKET* sock, SOCKADDR_IN* sock_addr, const uint16_t port, char* port_id) {
    int status;
    SOCKADDR_IN sockaddr;
    char host_name[100];
    gethostname(host_name, sizeof(host_name));
    struct hostent* remotehost = 0;
    struct in_addr addr;


    if (host_name == NULL)
    {
        printf("error initiating gethostname\n");
        strncpy_s(host_name, 8, "0.0.0.0\0", 8);
    }
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if ((*sock) == -1) {
        return STATUS_ERR_SOCK_CREATE;
    }
    remotehost = gethostbyname(host_name);
    if (remotehost == NULL) {
        printf("ERROR with remotehost name");
    }
    else {
        addr.s_addr = *(u_long*)remotehost->h_addr_list[0];
    }
    sockaddr.sin_addr.s_addr = inet_addr(inet_ntoa(addr)); //this line was derived by other students assignment
    sockaddr.sin_port = htons(port);
    sockaddr.sin_family = AF_INET;
    status = bind(*sock, (SOCKADDR_IN*)&sockaddr, sizeof(sockaddr));
    if (status == -1) {
        return STATUS_ERR_SOCK_BIND;
    }

    if ((listen(*sock, SOCKET_BACKLOG)) < 0) {
        return STATUS_ERR_SOCK_LISTEN;
    }
    if (port_id == PORT_SENDER)
    {
        printf("sender socket: IP Adress =  %s Port = %s\n", inet_ntoa(sockaddr.sin_addr), PORT_SENDER);
    }
    else if (port_id == PORT_RECEIVER)
    {
        printf("receiver socket: IP Adress =  %s Port = %s\n", inet_ntoa(sockaddr.sin_addr), PORT_RECEIVER);
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
    printf("connection with client failed, error %ld\n", WSAGetLastError()); // TODO - change line

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