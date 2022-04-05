// FileTransferChannel.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#define SA struct sockaddr
#include "FileTransferCommon/common.h"
#pragma warning(disable:4996)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

enum channel_mode_t { DETERMINISTIC = 0, RANDOM = 1, NONE = 2 };

void check_args(int argc, char* argv[]);
int is_number(char* string);
int fake_noise_random(char* buffer, double p, unsigned int seed);




int sender_OK = 0;

void sigpipe_handler()
{
	printf("SIGPIPE caught\n");
	sender_OK = 0;
}

int main(int argc, char* argv[])
{
	int status;
	SOCKET sockfd_sender = INVALID_SOCKET;
	SOCKET sockfd_recv = INVALID_SOCKET;
	int accept_res_sender, accept_res_recv, len_send, len_recv;
	struct sockaddr_in sender_addr, receiver_addr, channel_addr;
	WSADATA wsaData;
	enum channel_mode_t channel_mode;
	bit already_inserted_one_error = 0;

	char buf_tell_sender_to_start[1];
	buf_tell_sender_to_start[0] = 1;
	// safe_send(sockfd_sender, buf_tell_sender_to_start, 1);

	int addlen = sizeof(sockfd_sender);

	//==============intialize buffers for messagge and ack============
	char buffer[31];
	//char ack[100];

	//============initialize prameters for select function==============
	int sock_avl;
	uint64_t countTot = 0, cur_count = 0, flipped_bits = 0;
	struct timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = 50000;
	fd_set readfds, writefds;


	if (FLAG_DEBUG == 0) {
		check_args(argc, argv);
	}

	if (argc == 1) {
		channel_mode = NONE;
	}
	else if (strcmp(argv[1], "-r") == 0) {
		channel_mode = RANDOM;
	}
	else if (strcmp(argv[1], "-d") == 0) {
		channel_mode = DETERMINISTIC;
	}


	/////////---------------------Tx-RX flow ----------------------------------------------------------------------------------
	while (1) {

		if (socket_initialize(&wsaData) != NO_ERROR) {
			printf(MSG_ERR_WSASTARTUP);
			return 1;
		}
		sockfd_sender = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd_sender == INVALID_SOCKET){
			printf(MSG_ERR_CREATE_SOCK);
		}
		sockfd_recv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd_recv == INVALID_SOCKET) {
			printf(MSG_ERR_CREATE_SOCK);
		}
		status = socket_listen(&sockfd_sender, &channel_addr, CHANNEL_PORT_SENDER);
		if (status != STATUS_SUCCESS) {
			printf(MSG_ERR_SOCK_LISTEN, CHANNEL_PORT_SENDER);
			return status;
		}

		status = socket_listen(&sockfd_recv, &channel_addr, CHANNEL_PORT_RECEIVER);
		if (status != STATUS_SUCCESS) {
			printf(MSG_ERR_SOCK_LISTEN, CHANNEL_PORT_RECEIVER);
			return status;
		}

		// socket create and verification
		len_send = sizeof(&sender_addr);
		len_recv = sizeof(&receiver_addr);

		// Accept the data packet from client and verification
		// both sender and receiver must be connected at the same time in order for this to work!
	ACCEPT_SEND:
		accept_res_sender = accept(sockfd_sender, NULL, NULL);
	ACCEPT_RECV:
		accept_res_recv = accept(sockfd_recv, NULL, NULL);

		if (accept_res_sender < 0) {
			printf(MSG_ERR_SOCK_ACCEPT);
			goto ACCEPT_SEND;
		}

		if (accept_res_recv   < 0) {
			printf(MSG_ERR_SOCK_ACCEPT);
			goto ACCEPT_RECV;
		}


		if (1) {
				cur_count = 1;
				while (cur_count != 0) {
					cur_count = recv(accept_res_sender, &buffer, 31, 0);
					if (cur_count == SOCKET_ERROR) {
						printf(MSG_ERR_RECEIVE);
					}
					countTot += cur_count;
					if (channel_mode == RANDOM) {
						double probabilty = atoi(argv[2]) / pow(2, 16);
						if (probabilty > 1) probabilty = 1;
						flipped_bits += fake_noise_random(buffer, probabilty, atoi(argv[3]));
						if (send(accept_res_recv, buffer, cur_count, 0) == SOCKET_ERROR) {
							printf(MSG_ERR_SEND);
						}
					}
					else if (channel_mode == DETERMINISTIC)
					{
						flipped_bits += fake_noise_determ(buffer, argv[2]);
						if (send(accept_res_recv, buffer, cur_count, 0) == SOCKET_ERROR) {
							printf(MSG_ERR_SEND);
						}
					}
					else if (channel_mode == NONE) {
						// no noise
						//if (!already_inserted_one_error) {
						//	buffer[3] ^= 8;
						//	already_inserted_one_error = 1;
						//}
						flipped_bits += 0;
						send(accept_res_recv, buffer, cur_count, 0);
						if (send(accept_res_recv, buffer, cur_count, 0) == SOCKET_ERROR) {
							printf(MSG_ERR_SEND);
						}
					}
			}
			printf(MSG_TRANSMIT_CHANNEL, countTot, flipped_bits);

			closesocket(accept_res_sender);
			closesocket(accept_res_recv);
			closesocket(sockfd_sender);
			closesocket(sockfd_recv);
			char next;
			bit keep_question_loop = 1;
			bit should_break;
			while (keep_question_loop) {
				printf("Continue? (Y/N): ");
				scanf("%c", &next);
				if ((next=='Y') || (next=='y')) {
					should_break = 0;
					keep_question_loop = 0;
				} else if ((next=='N') || (next=='n')) {
					should_break = 1;
					keep_question_loop = 0;
				}
				else {
					keep_question_loop = 1;
					printf("Invalid response!\n");
				}
			}
			if (should_break == 1) break;
		}


	}
	//==================================closing sockets==========================
	closesocket(sockfd_sender);
	closesocket(sockfd_recv);
	exit(EXIT_SUCCESS);

}

void check_args(int argc, char* argv[])
{
	if (!((argc==3) || (argc==4))) {
		printf(MSG_ERR_ARGS_NUM);
		exit(EXIT_FAILURE);
	}
	if ((strcmp(argv[1], "-r") != 0) && (strcmp(argv[1], "-d") != 0)) {
		printf(MSG_ERR_ARGS_NOISE);
		exit(EXIT_FAILURE);
	}
	if (argc == 4) {
		if (!is_number(argv[3])) {
			printf(MSG_ERR_ARGS_SEED);
			exit(EXIT_FAILURE);
		}
		if (!is_number(argv[2])) {
			printf(MSG_ERR_ARGS_PROB);
		}
		if (atoi(argv[2]) > pow(2, 16) || atoi(argv[3]) < 0)
		{
			printf(MSG_ERR_ARGS_P_RANGE);
			exit(EXIT_FAILURE);
		}
	}
	else
		if (!is_number(argv[2]) && atoi(argv[2]) > 0)
		{
			printf(MSG_ERR_ARGS_CYCLE);
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
	for (int i = 0; i < strlen(buffer); i++)
	{
		for (int j = 0; j < 8; j++)
		{
			mask = pow(2, j);
			if (rand() % max_rand == 1)// a random lottery with the defined probability for every single bit
				if (flag == 0) {
					buffer[i] =buffer[i] ^ mask;
					count++;
				}
				else if (flag == 1 && rand() % 2 == 0) {
					buffer[i] = buffer[i] ^ mask;
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
	for (int i = 0; i < strlen(buffer); i++)
	{
		mask = 1;
		for (int j = 0; j < 8; i++)
		{
			if ((8 * i+j) % n == 0) {                 //other student in class helped me with this loop.
				buffer[i] = (char)(buffer[i] ^ mask);
				count++;
			}
		}
		mask *= 2;
	}
	return count;
}

