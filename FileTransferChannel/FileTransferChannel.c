// FileTransferChannel.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#define SA struct sockaddr
#include "FileTransferCommon/common.h"
#include "FileTransferCommon/common_includes.h"
#pragma warning(disable:4996)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

void check_args(int argc, char* argv[]);
int is_number(char* string);
int fake_noise_random(char* buffer, double p, unsigned int seed);
void open_socket(SOCKET* new_sock, struct sockaddr_in* sock_add, int port);

int main(int argc, char* argv[])
{
    check_args(argc, argv);
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
        printf("Error at WSAStartup()\n");
    SOCKET* sockfd_sender = NULL;
    SOCKET* sockfd_recv = NULL;
    int accept_res_sender, accept_res_recv, len_send, len_recv;
    struct sockaddr_in sender_addr, receiver_addr, channel_addr;
    open_socket(sockfd_sender, &channel_addr, 3642);
    open_socket(sockfd_recv, &channel_addr, 3643);

    // socket create and verification

    len_send = sizeof(&sender_addr);
    len_recv = sizeof(&receiver_addr);

    // Accept the data packet from client and verification
    accept_res_sender = accept(sockfd_sender, (SA*)&sender_addr, &len_send);
    accept_res_recv = accept(sockfd_recv, (SA*)&receiver_addr, &len_recv);

    if (accept_res_sender < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
    if (accept_res_recv < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    int addlen = sizeof(sockfd_recv);

    //==============intialize buffers for messagge and ack============
    char buffer[2040];
    char ack[100];

    //============initialize prameters for select function==============
    int sock_avl, countTot = 0, cur_count = 0, flipped_bits = 0;
    struct timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = 50000;
    fd_set readfds, writefds;
    /////////---------------------Tx-RX flow ----------------------------------------------------------------------------------
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd_sender, &readfds);
        FD_SET(sockfd_recv, &readfds);
        sock_avl = select(sockfd_recv + 1, &readfds, NULL, NULL, &tm);

        if (sock_avl < 0) {
            printf("select error. Error\n");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(sockfd_sender, &readfds)) { //receiving data from client
            cur_count = recvfrom(sockfd_sender, buffer, 2040, 0, (struct sockaddr*)&sockfd_sender, &addlen);
            countTot += cur_count;
            if (argv[1] == "-r") {
                double probabilty = atoi(argv[2]) / pow(2, 16);
                if (probabilty > 1) probabilty = 1;
                flipped_bits += fake_noise_random(buffer, probabilty, atoi(argv[3]));
                sendto(sockfd_recv, buffer, cur_count, 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
            }
            else if (argv[1] == "-d")
            {
                flipped_bits += fake_noise_determ(buffer, argv[2]);
                sendto(sockfd_recv, buffer, cur_count, 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
            }
            fprintf(stderr, "sender socket: IP address = 0.0.0.0 port = 6342\nreceiver socket: IP address = 0.0.0.0 port = 6343\nretransmittes %d bytes, flipped %d bits", countTot, flipped_bits);
            fprintf(stderr, "continue? (yes/no)\n");
            char* next = NULL;
            sscanf("%s", next);
            if (next == "no") break;
        }


    }
    //==================================closing sockets==========================
    closesocket(sockfd_sender);
    closesocket(sockfd_recv);
    exit(EXIT_SUCCESS);

}

void check_args(int argc, char* argv[])
{
    if ((argc != 3) && (argc != 4)) {
        perror("wrong number of arguments, channel requiers 2 or 3 arguments.");
        exit(EXIT_FAILURE);
    }
    if ((strcmp(argv[1], "-r") != 0) && (strcmp(argv[1], "-d") != 0)) {
        perror("wrong argument, must choose the noise method");
        exit(EXIT_FAILURE);
    }
    if (argc == 4) {
        if (!is_number(argv[3])) {
            perror("random seed has to be a number");
            exit(EXIT_FAILURE);
        }
        if (!is_number(argv[2])) {
            perror("probabilty has to be a number");
        }
        if (atoi(argv[2]) > pow(2, 16) || atoi(argv[3]) < 0)
        {
            perror("p/2^16 is in range 0-1");
            exit(EXIT_FAILURE);
        }
    }
    else
        if (!is_number(argv[2]) && atoi(argv[2]) > 0)
        {
            perror("length of cycle nust be positive integer");
            exit(EXIT_FAILURE);
        }
}

//checking is string is a number
int is_number(char* string)
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] < '0' || string[i]>'9')
            return 0;
    }
    return 1;
}

//flip bits randomly with a given probabilty by the given seed
int fake_noise_random(char* buffer, double p, unsigned int seed) {
    srand(seed);
    int mask, max_rand = 1 / p, count = 0;
    int flag = (p == 1 / pow(2, 16));
    for (int i = 0; i < 2040; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            mask = pow(2, j);
            if (rand() % max_rand == 1)// a random lottery with the defined probability for every single bit
                if (flag == 0) {
                    buffer[i] = (char)(buffer[i] ^ mask);
                    count++;
                }
                else if (flag == 1 && rand() % 2 == 0) {
                    buffer[i] = (char)(buffer[i] ^ mask);
                    count++;
                }

        }
    }
    return count;
}

int fake_noise_determ(char* buffer, int n)
{
    int mask = 1;
    int count = 0;
    for (int i = 0; i < 2040; i++)
    {
        if (i % n == 0) {
            buffer[i] = (char)(buffer[i] ^ mask);
            count++;
        }
    }
    return count;
}

void open_socket(SOCKET* new_sock, struct sockaddr_in* sock_add, int port)
{
   *new_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*new_sock == -1) {
        printf("socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully created..\n");
    sock_add->sin_family = AF_INET;
    sock_add->sin_addr.s_addr = htonl(INADDR_ANY);
    sock_add->sin_port = htons(port);
    if ((bind(*new_sock, (SA*)&sock_add, sizeof(sock_add))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
    if ((listen(*new_sock, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    return;
}
