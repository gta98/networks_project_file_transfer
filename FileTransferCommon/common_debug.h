#pragma once

#define FLAG_DEBUG         0
#define FLAG_IGNORE_SOCKET 0
#define FLAG_SKIP_FILENAME 0
#define FLAG_SINGLE_ITER   0
#define FLAG_HAMMING_DIS   0

#define DEBUG_FILE_PATH      "C:\\Users\\tommy\\source\\networks_project_file_transfer\\x64\\Debug\\myfile.txt"
#define DEBUG_FILE_PATH_RECV "C:\Users\tommy\source\networks_project_file_transfer\x64\Debug\\myfile.txt"

#if FLAG_DEBUG == 1
#define printd printf
#else
#define printd(...)
#endif

#define perror(...) fprintf(stderr, __VA_ARGS__)

#define _CRT_SECURE_NO_WARNINGS