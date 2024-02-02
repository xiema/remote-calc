#include "server.h"
#include "socket.h"
#include "calc.h"

#include <stdio.h>
#include <winsock2.h>

int terminate = 0;

BOOL CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        terminate = 1;
        return TRUE; // Handler processed the CTRL+C signal
    }
    return FALSE; // Let the default handler process the signal
}

int server_loop(SOCKET new_socket) {
    int recv_size, val, err;
    char reply[APP_MAXCMDLENGTH+1];
    char msg[APP_MAXCMDLENGTH + 1];

    while (1) {
        err = 0;
        if ((recv_size = recv(new_socket, msg, APP_MAXCMDLENGTH, 0)) == SOCKET_ERROR) {
            puts("Client disconnected");
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
}

int server_run() {
    SOCKET s;
    struct sockaddr_in server, client;

    // Setup SIGINT handler
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) {
        fprintf(stderr, "Failed to set up signal handler\n");
        return 1;
    }

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
    int result;

    while (!terminate) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(s, &readSet);

        struct timeval timeout;
        timeout.tv_sec = 1;  // Set the timeout to 1 second
        timeout.tv_usec = 0;

        result = select(0, &readSet, NULL, NULL, &timeout);
        if (result == SOCKET_ERROR) {
            fprintf(stderr, "Select failed: %d\n", WSAGetLastError());
            break;
        }

        if (result > 0) {
            // There is activity on the socket, accept the connection
            SOCKET clientSocket = accept(s, (struct sockaddr *)&client, &c);
            if (clientSocket == INVALID_SOCKET) {
                fprintf(stderr, "Accept failed: %d\n", WSAGetLastError());
                break;
            }
            puts("Client connected");
            server_loop(clientSocket);
            // Close the client socket
            closesocket(clientSocket);
        }
    }

    puts("Quitting...");

    // Close sockets
    closesocket(s);

    // Socket library cleanup
    sockQuit();

    return 0;
}