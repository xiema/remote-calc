#include "calc.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ALLOCATED = 0;
#ifdef TEST
void* customMalloc(size_t size) {
    ALLOCATED++;
    return malloc(size);
}
void customFree(void* ptr) {
    free(ptr);
    ALLOCATED--;
}
#else
#define malloc customMalloc
#define free customFree
#endif

int calc_run() {
    char cmd[APP_MAXCMDLENGTH+1];

    // TODO: check behavior for multiple lines
    fgets(cmd, APP_MAXCMDLENGTH, stdin);

    parse_state ps = { .str = cmd, .curpos = 0, .maxlength = APP_MAXCMDLENGTH };

    printf(cmd);

    return 0;
}

int parse_expression(parse_state ps, expression* out) {
    return 0;
}

expression* create_number(int value) {
    expression* expr = customMalloc(sizeof(expression));
    // expression* expr = malloc(sizeof(expression));
    expr->type = EXPR_NUMBER;
    (expr->data).value = value;
    return expr;
}

expression* create_tree(char op, expression* left, expression* right) {
    expression* expr = customMalloc(sizeof(expression));
    // expression* expr = malloc(sizeof(expression));
    expr->type = EXPR_TREE;
    (expr->data).tree.op = op;
    (expr->data).tree.left = left;
    (expr->data).tree.right = right;
    return expr;
}

int free_expression(expression* expr) {
    if (expr->type == EXPR_TREE) {
        free_expression((expr->data).tree.left);
        free_expression((expr->data).tree.right);
    }
    // free(expr);
    customFree(expr);
}

/* Convert an expression into a human-readable string. Returns the length of the 
   resulting string on success. Returns a negative number upon error.
*/
int debug_expression(expression* expr, char* out, int curpos, int maxlength) {
    if (expr->type == EXPR_NUMBER) {
        char numstr[APP_MAXNUMLENGTH];
        int len;
        sprintf(numstr, "%d", (expr->data).value);
        len = strlen(numstr);
        if (len < maxlength - curpos) {
            strcpy(out+curpos, numstr);
            return len;
        }
        else {
            return -1;
        }
    }
    else if (expr->type == EXPR_TREE) {
        int len = 0;

        if (curpos+len == maxlength) return -1;
        out[curpos+(len++)] = '(';

        int adv;
        adv = debug_expression((expr->data).tree.left, out, curpos+len, maxlength);
        if (adv < 0) return -1;
        len += adv;

        if (curpos+len == maxlength) return -1;
        out[curpos+(len++)] = (expr->data).tree.op;

        adv = debug_expression((expr->data).tree.right, out, curpos+len, maxlength);
        if (adv < 0) return -1;
        len += adv;

        if (curpos+len == maxlength) return -1;
        out[curpos+(len++)] = ')';

        return len;
    }
    else {
        return -1;
    }
}