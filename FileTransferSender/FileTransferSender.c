// FileTransferSender.c : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "winsock2.h"

#define FLAG_DEBUG

#ifdef FLAG_DEBUG
#define printd printf
#else
#define printd(...)
#endif

#define bit unsigned char

// NOTE: parity32, hamming_encode, hamming_decode, print_bin taken from:
// https://gist.github.com/qsxcv/b2f9976763d52bf1e7fc255f52f05f5b

// https://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
static inline bool parity32(uint32_t v)
{
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}

// Hamming(31, 26) plus total parity at bit 0 for double error detection
// https://en.wikipedia.org/wiki/Hamming_code#General_algorithm
uint32_t hamming_encode(uint32_t d)
{
    // move data bits into position
    uint32_t h =
        (d & 1) << 3 |
        (d & ((1 << 4) - (1 << 1))) << 4 |
        (d & ((1 << 11) - (1 << 4))) << 5 |
        (d & ((1 << 26) - (1 << 11))) << 6;
    // compute parity bits
    h |=
        parity32(h & 0b10101010101010101010101010101010) << 1 |
        parity32(h & 0b11001100110011001100110011001100) << 2 |
        parity32(h & 0b11110000111100001111000011110000) << 4 |
        parity32(h & 0b11111111000000001111111100000000) << 8 |
        parity32(h & 0b11111111111111110000000000000000) << 16;
    // overall parity
    return h | parity32(h);
}

uint32_t hamming_decode(uint32_t h)
{
    // overall parity error
    bool p = parity32(h);
    // error syndrome
    uint32_t i =
        parity32(h & 0b10101010101010101010101010101010) << 0 |
        parity32(h & 0b11001100110011001100110011001100) << 1 |
        parity32(h & 0b11110000111100001111000011110000) << 2 |
        parity32(h & 0b11111111000000001111111100000000) << 3 |
        parity32(h & 0b11111111111111110000000000000000) << 4;
    // correct single error or detect double error
    if (i != 0) {
        if (p == 1) { // single error
            h ^= 1 << i;
        }
        else { // double error
            return ~0;
        }
    }
    // remove parity bits
    return
        ((h >> 3) & 1) |
        ((h >> 4) & ((1 << 4) - (1 << 1))) |
        ((h >> 5) & ((1 << 11) - (1 << 4))) |
        ((h >> 6) & ((1 << 26) - (1 << 11)));
}

void print_bin(uint32_t v)
{
    for (int i = 31; i >= 0; i--)
        printf("%d", (v & (1 << i)) >> i);
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