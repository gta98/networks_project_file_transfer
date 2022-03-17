#pragma once
#ifndef H_COMMON_SOCKET_UTILS
#define H_COMMON_SOCKET_UTILS

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "common_includes.h"

extern boolean socket_initialize(WSADATA* wsaData);
int socket_connect(SOCKET* sock, const char* dest, const u_short port);
#endif