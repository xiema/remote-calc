#include "socket.h"

#include <winsock2.h>

int sockInit(void) {
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1,1), &wsa_data);
}

int sockQuit(void) {
    return WSACleanup();
}