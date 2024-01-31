#include "server.h"
#include "client.h"
#include "calc.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) return 1;
    if (strcmp(argv[1], "server") == 0) {
        server_run();
    }
    else if (strcmp(argv[1], "client") == 0) {
        client_run();
    }
    else if (strcmp(argv[1], "calc") == 0) {
        calc_run();
    }
    return 0;
}