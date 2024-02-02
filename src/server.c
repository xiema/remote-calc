#include "server.h"
#include "socket.h"
#include "calc.h"

#include <stdio.h>
#include <winsock2.h>


// TODO: Ensure cleanup
int server_run() {
    SOCKET s;
    struct sockaddr_in server, client;
    char msg[APP_MAXCMDLENGTH + 1];

    // Socket library initialization
    if (sockInit() != 0) {
        printf("Socket initialization failed: Error code %d", WSAGetLastError());
        return 1;
    }
    
    // Set connection details
    // TODO: Allow other address/port
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(APP_DEFAULTPORT);
    
    // Create socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation error: %d", WSAGetLastError());
        return 1;
    }

    // Bind
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed: Error code %d", WSAGetLastError());
    }

    // Listen
    listen (s, 3);

    int c = sizeof(struct sockaddr_in);
    SOCKET new_socket = accept(s, (struct sockaddr *)&client, &c);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept connection failed: Error code %d", WSAGetLastError());
    }

    int recv_size, val, err;
    char reply[APP_MAXCMDLENGTH+1];
    while (1) {
        err = 0;
        if ((recv_size = recv(new_socket, msg, APP_MAXCMDLENGTH, 0)) == SOCKET_ERROR) {
            puts("Receive failed");
            break;
        }
        else {
            msg[recv_size] = '\0';
            printf("Received command: %s\n", msg);
            val = calculate_value_str(msg, &err);
            if (err != 0) {
                sprintf(reply, "Error %d", err);
                puts(reply);
            }
            else {
                sprintf(reply, "%d", val);
                printf("Sending: %d\n", val);
            }
            if (send(new_socket, reply, strlen(reply), 0) < 0) {
                puts("Send reply failed");
            }
        }
    }

    // Close sockets
    closesocket(s);
    closesocket(new_socket);

    // Socket library cleanup
    sockQuit();

    return 0;
}