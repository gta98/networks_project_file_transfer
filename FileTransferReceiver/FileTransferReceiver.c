// FileTransferSender.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include "FileTransferCommon/common.h"

int socket_recv_file(const SOCKET* sock, const char* file_name, ull* file_size, ull* file_total_recv) {
    char buf_send[4], buf_hold[1], buf_encode[4], buf_read[4], buf_recv_enc[31];
    char* buf_recv_raw;
    int status;
    errno_t err;
    FILE* fp;

    *file_size = 0;
    *file_total_recv = 0;

    int bytes_missing_for_26 = 0;// (26 - (1 + 8 + (*(file_size)) % 26)) % 26;
    int total_bytes_added = 0;
    int buf_size = 0;// (*file_size) + total_bytes_added;

    uint64_t transmission_size = 0;// 31 * m;

    /*buf_read[0] = total_bytes_added;
    buf_read[1] = (0b1111111100000000000000000000000000000000000000000000000000000000 & expected_transmission_size) >> 56;
    buf_read[2] = (0b0000000011111111000000000000000000000000000000000000000000000000 & expected_transmission_size) >> 48;
    buf_read[3] = (0b0000000000000000110000000000000000000000000000000000000000000000 & expected_transmission_size) >> 46;


    send(sock, buf_read, sizeof(char) * 4, 0);*/
    for (int i = 0; i < 31; i++) {
        buf_hold[0] = 0;
        status = recv(sock, buf_hold, 1, 0);
        if (status == -1) {
            i -= 1;
            continue;
        }
        buf_recv_enc[i] = buf_hold[0];
    }
    buf_recv_raw = NULL;
    decode_31_block_to_26(&buf_recv_raw, buf_recv_enc, 31);

    total_bytes_added = buf_recv_raw[0];

    // classic buffer overflow risk
    transmission_size  = 0;
    transmission_size |= buf_recv_raw[1] << 56;
    transmission_size |= buf_recv_raw[2] << 48;
    transmission_size |= buf_recv_raw[3] << 40;
    transmission_size |= buf_recv_raw[4] << 32;
    transmission_size |= buf_recv_raw[5] << 24;
    transmission_size |= buf_recv_raw[6] << 16;
    transmission_size |= buf_recv_raw[7] <<  8;
    transmission_size |= buf_recv_raw[8] <<  0;
    *file_total_recv = transmission_size;

    int total_bytes_not_recv = transmission_size - 31;
    int actual_size = transmission_size - total_bytes_added;

    free(buf_recv_raw);
    buf_recv_raw = NULL;

    // yeah sure load it all to memory, why the heck not
    int attempts;
    attempts = 10; // :)
    char* buf = NULL;// malloc((*file_size) * sizeof(char));
    while (buf == NULL && attempts > 0) { // :)
        buf = malloc(transmission_size);
        //return 2;
        attempts--;
    }
    if (attempts == -1) return STATUS_ERR_MALLOC_BUF;

    for (int i = 0; i < transmission_size; i++) buf[i] = 0;

    // classic buffer overflow risk
    for (int i = 0; i < 31; i++) buf[i] = buf_recv_enc[i];

    printf("buf: ");
    for (int i = 0; i < 31; i++) {
        printf("%x ", buf[i]);
    }
    printf("\n");

    int idx_in_buf = 32;
    while (idx_in_buf < transmission_size) {
        buf_hold[0] = 0;
        status = recv(sock, buf_hold, 1, 0);
        if (status != -1) {
            buf[idx_in_buf] = buf_hold[0];
            idx_in_buf += 1;
        }
    }

    buf_recv_raw = NULL;
    uint64_t raw_size = decode_31_block_to_26(&buf_recv_raw, buf, transmission_size);
    *file_size = raw_size;

    printf("buf: ");
    for (int i = 0; i < raw_size; i++) {
        printf("%x ", buf_recv_raw[i]);
    }
    printf("\n");

    if (fopen_s(&fp, file_name, "w") != 0) {
        return STATUS_ERR_FILE_READ;
    }
    fprintf(fp, buf_recv_raw+total_bytes_added);

    free(buf);
    free(buf_recv_raw);
    fclose(fp);
    return 0;
}

int main(const int argc, const char* argv[])
{
    char* remote_addr;
    u_short remote_port;
    SOCKET sock;
    WSADATA wsaData;
    int status;

    char file_name[MAX_PERMITTED_FILE_PATH_LENGTH];
    ull file_size, file_total_recv;

    if (argc != 3) {
        remote_addr = CHANNEL_ADDR;
        remote_port = CHANNEL_PORT_RECEIVER;
        printd("WARNING: proper syntax is as follows:\n");
        printd("         %s IP PORT\n", argv[0]);
        printd("         an invalid number of arguments was specified, so using IP=%s, PORT=%d\n", remote_addr, remote_port);
    }
    else {
        remote_addr = argv[1];
        remote_port = (u_short)atoi(argv[2]);
    }

    if (socket_initialize(&wsaData) != NO_ERROR) {
        printf(MSG_ERR_WSASTARTUP);
        return 1;
    }

    printd("Attempting connection to %s:%d\n", remote_addr, remote_port);
    status = socket_connect(&sock, remote_addr, remote_port);
    if (status == SOCKET_ERROR) {
        printf(MSG_ERR_CONNECTING, status, remote_addr, remote_port);
#if FLAG_IGNORE_SOCKET != 1
        return 1;
#endif
    }
    printd("Connected to %s:%d\n", remote_addr, remote_port);


    while (1) {
        printf(MSG_ENTER_FILENAME);
#if FLAG_SKIP_FILENAME==1
        strncpy_s(file_name, 100, DEBUG_FILE_PATH_RECV, strlen(DEBUG_FILE_PATH_RECV));
#else
        scanf_s("%s", file_name, MAX_PERMITTED_FILE_PATH_LENGTH);
        if (strcmp(file_name, "quit") == 0) {
            return 0;
        }
#endif

        printd("Receiving file...\n");
        status = socket_recv_file(sock, file_name, &file_size, &file_total_recv);
        switch (status) {
        case STATUS_SUCCESS: {
            printf(MSG_FILE_LENGTH, file_size);
            printf(MSG_TOTAL_SENT, file_total_recv);
            break;
        }
        case STATUS_ERR_FILE_READ: {
            printf(MSG_ERR_FILE_READ, file_name);
            break;
        }
        case STATUS_ERR_MALLOC_BUF: {
            printf(MSG_ERR_MALLOC_BUF, file_size);
            break;
        }
        case STATUS_ERR_MALLOC_BUF_ENC: {
            printf(MSG_ERR_MALLOC_BUF_ENC, file_total_recv);
            break;
        }
        case STATUS_ERR_BUF_SIZE: {
            printf(MSG_ERR_BUF_SIZE, file_size);
        }
        default: {
            printf(MSG_ERR_UNKNOWN, status);
            break;
        }
        }
#if FLAG_SINGLE_ITER==1
        break;
#endif
    }

    return 0;
}