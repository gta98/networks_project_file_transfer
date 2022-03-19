// FileTransferChannel.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include "FileTransferCommon/common.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

void check_args(int argc, char* argv[]);
int is_number(char* string);
int fake_noise_random(char* buffer, double p, unsigned int seed);

int main(int argc, char* argv[])
{
    check_args(argc, argv);
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
        printf("Error at WSAStartup()\n");

    SOCKET client_sock = 0, server_sock = 0;
    struct sockaddr_in server_add, client_add, channel_add;
    //TODO - open 2 sockets, gethostbyname for IPV4
    int addlen = sizeof(client_add);

} // my comment    v 

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