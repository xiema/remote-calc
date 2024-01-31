#include "calc.h"
#include "socket.h"

#include <stdio.h>

typedef struct expression {
    char op;
    int val;
    struct expression *left;
    struct expression *right;
} expression;

int calc_run() {
    char cmd[APP_MAXCMDLENGTH+1];

    // TODO: check behavior for multiple lines
    fgets(cmd, APP_MAXCMDLENGTH, stdin);

    printf(cmd);

    return 0;
}

