#include "client.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

int client_run(char* ip_addr) {
    SOCKET s;
    struct sockaddr_in server;

    // Socket library initialization
    if (sockInit() != 0) {
        printf("Socket initialization failed: Error code %d", WSAGetLastError());
        return 1;
    }
    
    // Set connection details
    // TODO: Allow other address/port
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons(APP_DEFAULTPORT);
    
    // Create socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation error: %d", WSAGetLastError());
        return 1;
    }

    // Connect to server
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        puts("Connection error");
        return 1;
    }

    char cmd[APP_MAXCMDLENGTH+1];
    char recv_msg[APP_MAXCMDLENGTH+1];
    int err, recv_size;
    while (1) {
        fgets(cmd, APP_MAXCMDLENGTH, stdin);
        err = 0;;
        if (cmd[strlen(cmd)-1] == '\n') cmd[strlen(cmd)-1] = '\0';
        if (send(s, cmd, strlen(cmd), 0) < 0) {
            puts("Send message failed");
            continue;
        }
        
        if ((recv_size = recv(s, recv_msg, APP_MAXCMDLENGTH, 0)) == SOCKET_ERROR) {
            puts ("Receive failed");
        }
        else {
            recv_msg[recv_size] = '\0';
            printf("Received: %s\n", recv_msg);
        }
    }

    // Close socket
    closesocket(s);

    // Socket library cleanup
    sockQuit();

    return 0;
}