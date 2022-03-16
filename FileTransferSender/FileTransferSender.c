// FileTransferSender.c : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "stdio.h"
#include "winsock2.h"

#define FLAG_DEBUG

#ifdef FLAG_DEBUG
#define printd printf
#else
#define printd(...)
#endif

#define bit unsigned char

bit select_bit(u_int x, u_int n) {
    return (x >> (n-1)) & 1;
}

u_int encode_31_26_3(u_int data) {
    u_int encoded = 0;
    u_int parity_bit_1, parity_bit_2, parity_bit_4, parity_bit_8, parity_bit_16;

    // data bits
    encoded = data;
    encoded = ((0xFFFFFFFF & encoded) << 2);
    encoded = ((0xFFFFFFF8 & encoded) << 1) | ((0xFFFFFFFF - 0xFFFFFFF8) & encoded);
    encoded = ((0xFFFFFF80 & encoded) << 1) | ((0xFFFFFFFF - 0xFFFFFF80) & encoded);
    encoded = ((0xFFFF8000 & encoded) << 1) | ((0xFFFFFFFF - 0xFFFF8000) & encoded);

    // parity bits
    int parity_power, parity_bit_location, parity_bit, data_bit_location;
    for (parity_power = 0; parity_power <= 4; parity_power++) {
        parity_bit = 0;
        parity_bit_location = 1 << parity_power;
        for (data_bit_location = 0; data_bit_location <= 30; data_bit_location++) {
            if ((((data_bit_location+1) & parity_bit_location) != 0) && (((data_bit_location + 1) & parity_bit_location) != parity_bit_location)) {
                parity_bit ^= 1&(encoded >> data_bit_location);
            }
        }
        encoded = encoded | (parity_bit << (parity_bit_location - 1));
    }

    return encoded;
}

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
    char*   remote_addr;
    u_short remote_port;
    SOCKET sock;
    WSADATA wsaData;
    int status;

    char* file_name;
    int file_size, file_total_sent;

    if (argc != 3) {
        remote_addr = "127.0.0.1";
        remote_port = (u_short) 3490;
        printd("WARNING: proper syntax is as follows:\n");
        printd("         %s IP PORT\n", argv[0]);
        printd("         an invalid number of arguments was specified, so using IP=%s, PORT=%d\n", remote_addr, remote_port);
    } else {
        remote_addr = argv[1];
        remote_port = (u_short) atoi(argv[2]);
    }

    if (socket_initialize(&wsaData) != NO_ERROR) {
        printd("Error at WSAStartup()\n");
        return 1;
    }

    printd("Attempting connection to %s:%d\n", remote_addr, remote_port);
    status = socket_connect(&sock, remote_addr, remote_port);
    if (status == SOCKET_ERROR) {
        printd("Error (%d) connecting to %s:%d\n", status, remote_addr, remote_port);
        return 1;
    }
    printd("Connected to %s:%d\n", remote_addr, remote_port);


    while (0) {
        printf("Enter filename: ");
        scanf_s("%s", &file_name);
        if (strcmp(file_name, "quit") == 0) {
            return 0;
        }

        printf("Sending file...\n");
        //socket_send_file(file_name, &file_size, &file_total_sent);
        printf("File length: %d\n", file_size);
        printf("Bytes sent:  %d\n", file_total_sent);
    }

    return 0;
}