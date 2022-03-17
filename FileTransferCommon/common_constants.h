#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define STATUS_SUCCESS            0
#define STATUS_ERR_FILE_READ      1
#define STATUS_ERR_MALLOC_BUF     2
#define STATUS_ERR_MALLOC_BUF_ENC 3
#define STATUS_ERR_BUF_SIZE       4

#define MSG_ERR_WSASTARTUP     "Error at WSAStartup()\n"
#define MSG_ERR_CONNECTING     "Error (%d) connecting to %s:%d\n"
#define MSG_ENTER_FILENAME     "Enter filename: "
#define MSG_FILE_LENGTH        "File length: %llu bytes\n"
#define MSG_TOTAL_SENT         "Total sent:  %llu bytes\n"
#define MSG_ERR_FILE_READ      "ERROR: Could not read file %s\n"
#define MSG_ERR_MALLOC_BUF     "ERROR: Could not allocate %llu bytes of memory required for storing file content\n"
#define MSG_ERR_MALLOC_BUF_ENC "ERROR: Could not allocate %llu bytes of memory required for storing encoded file content\n"

#define MAX_FILE_PATH_LENGTH           32767
#define MAX_PERMITTED_FILE_PATH_LENGTH 4096