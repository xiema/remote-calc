#include "server.h"
#include "client.h"
#include "calc.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    if (strcmp(argv[1], "server") == 0) {
        return server_run();
    }
    else if (strcmp(argv[1], "client") == 0) {
        if (argc > 2) {
            return client_run(argv[2]);
        }
        else {
            return 1;
        }
    }
    else if (strcmp(argv[1], "calc") == 0) {
        return calc_run();
    }
}