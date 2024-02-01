#include "calc.h"
#include "socket.h"

#include "sc_queue.h"

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
    struct sc_queue_char stack;
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
        (expr->data).tree.left = NULL;
        free_expression((expr->data).tree.right);
        (expr->data).tree.right = NULL;
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

static int is_digit(char c) {
    char* DIGITS = "0123456789";
    for (int i = 0; i < 10; i++) {
        if (c == DIGITS[i]) return i;
    }
    return -1;
}

int tokenize(char* str, token* out, int maxlength) {
    int state = 0; //0 - symbol; 1 - number
    int len = 0;
    int d;
    for (int curpos = 0; curpos < maxlength; curpos++) {
        // FINISH
        if (str[curpos] == '\0') {
            if (state == 1) {
                len++;
                state = 0;
            }
            break;
        }
        // DIGIT
        else if ((d = is_digit(str[curpos])) >= 0) {
            if (state == 0) {
                out[len].type = TOK_NUMBER;
                out[len].val = 0;
            }
            out[len].val = out[len].val * 10 + d;
            state = 1;
        }
        // PAREN
        else if (str[curpos] == '(' || str[curpos] == ')') {
            if (state == 1) {
                len++;
                state = 0;
            }
            out[len].type = TOK_PAREN;
            out[len].val = (int)(str[curpos]);
            len++;
        }
        // OP
        else if (str[curpos] == '+' || str[curpos] == '-' || str[curpos] == '*' || str[curpos] == '/') {
            if (state == 1) {
                len++;
                state = 0;
            }
            out[len].type = TOK_OP;
            out[len].val = (int)(str[curpos]);
            len++;
        }
        // WHITESPACE
        else if (str[curpos] == ' ') {
            if (state == 1) {
                len++;
                state = 0;
            }
            continue;
        }

        if (len >= APP_MAXTOKENLENGTH) return -1;
    }

    return len;
}