// FileTransferSender.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include "FileTransferCommon/common.h"

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 3490

int socket_send_file(const SOCKET* sock, const char* file_name, ull* file_size, ull* file_total_sent) {
    char buf_send[4], buf_hold[1];
    errno_t err;
    FILE* fp;

    *file_size       = 0;
    *file_total_sent = 0;

    if (fopen_s(&fp, file_name, "r") != 0) {
        return STATUS_ERR_FILE_READ;
    }

    fseek(fp, 0, SEEK_END);
    *file_size = ftell(fp); // bytes
    // first byte will tell us how many bytes were added to the actual data
    // next 8 bytes will tell us the size of the transmission
    fseek(fp, 0, SEEK_SET);

    int bytes_missing_for_26 = (26 - (1+8+(*(file_size)) % 26)) % 26;
    int total_bytes_added = 1 + 8 + bytes_missing_for_26;
    int buf_size = (*file_size) + total_bytes_added;

    // yeah sure load it all to memory, why the heck not
    int attempts;
    attempts = 10; // :)
    char* buf = NULL;// malloc((*file_size) * sizeof(char));
    while (buf == NULL && attempts > 0) { // :)
        buf = malloc(buf_size * sizeof(char));
        //return 2;
        attempts--;
    }
    if (attempts == -1) return 2;

    for (int i = 0; i < total_bytes_added; i++) buf[i] = 0;
    buf[0] = total_bytes_added;
    for (int i = total_bytes_added; i < buf_size; i++) {
        fread(buf_hold, sizeof(char), 1, fp);
        buf[i] = buf_hold[0];
    }
    
    // now, encode and stream it!
    // buf_size = 26m, => m=buf_size/26, new_buf_size=31*(buf_size/26)
    *file_total_sent = 31 * (buf_size / 26);
    attempts = 10; // :)
    char* buf_enc = NULL;// malloc((*file_size) * sizeof(char));
    while (buf_enc == NULL && attempts > 0) { // :)
        buf_enc = malloc((*file_total_sent) * sizeof(char));
        //return 2;
        attempts--;
    }
    if (attempts == -1) return 3;

    //return 0;

    /*send(sock, buf_send, 1, 0);


    buf_send[0] = bytes_missing_for_26;
    send(sock, buf_send, 1, 0);*/



    free(buf);
    free(buf_enc);
    fclose(fp);
    return 0;
}

int main(const int argc, const char *argv[])
{
    char*   remote_addr;
    u_short remote_port;
    SOCKET sock;
    WSADATA wsaData;
    int status;

    char file_name[MAX_PERMITTED_FILE_PATH_LENGTH];
    ull file_size, file_total_sent;

    if (argc != 3) {
        remote_addr = DEFAULT_HOST;
        remote_port = DEFAULT_PORT;
        printd("WARNING: proper syntax is as follows:\n");
        printd("         %s IP PORT\n", argv[0]);
        printd("         an invalid number of arguments was specified, so using IP=%s, PORT=%d\n", remote_addr, remote_port);
    } else {
        remote_addr = argv[1];
        remote_port = (u_short) atoi(argv[2]);
    }

    if (socket_initialize(&wsaData) != NO_ERROR) {
        printf(MSG_ERR_WSASTARTUP);
        return 1;
    }

    printd("Attempting connection to %s:%d\n", remote_addr, remote_port);
    status = socket_connect(&sock, remote_addr, remote_port);
    if (status == SOCKET_ERROR) {
        printf(MSG_ERR_CONNECTING, status, remote_addr, remote_port);
        return 1;
    }
    printd("Connected to %s:%d\n", remote_addr, remote_port);


    while (1) {
        printf(MSG_ENTER_FILENAME);
        scanf_s("%s", file_name, MAX_PERMITTED_FILE_PATH_LENGTH);
        if (strcmp(file_name, "quit") == 0) {
            return 0;
        }

        printd("Sending file...\n");
        status = socket_send_file(sock, file_name, &file_size, &file_total_sent);
        switch (status) {
            case STATUS_SUCCESS: {
                printf(MSG_FILE_LENGTH, file_size);
                printf(MSG_TOTAL_SENT,  file_total_sent);
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
                printf(MSG_ERR_MALLOC_BUF_ENC, file_total_sent);
                break;
            }
        }
        
    }

    return 0;
}