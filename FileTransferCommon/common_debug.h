#pragma once

#define FLAG_DEBUG         1
#define FLAG_IGNORE_SOCKET 1
#define FLAG_SKIP_FILENAME 1
#define FLAG_SINGLE_ITER   1

#define DEBUG_FILE_PATH "C:\\Users\\adhoms\\Desktop\\hr.txt"

#if FLAG_DEBUG == 1
#define printd printf
#else
#define printd(...)
#endif