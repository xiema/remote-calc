#include "client.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

int get_send_command(SOCKET s) {
    char cmd[APP_MAXCMDLENGTH+1];

    fgets(cmd, sizeof(cmd), stdin);

    return 0;
}

// TODO: Ensure cleanup
int client_run() {
    SOCKET s;
    struct sockaddr_in server;

    // Socket library initialization
    if (sockInit() != 0) {
        printf("Socket initialization failed: Error code %d", WSAGetLastError());
        return 1;
    }
    
    // Set connection details
    // TODO: Allow other address/port
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
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

    char *message = "This is a message!";
    if (send(s, message, strlen(message), 0) < 0) {
        puts("Send message failed");
    }
    else {
        puts("Message sent\n");
    }

    // Close socket
    closesocket(s);

    // Socket library cleanup
    sockQuit();

    return 0;
}