#include "calc.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* For debugging purposes, can compile with custom allocation/deallocation
   functions that allow tracking of the allocated object count in the 
   global variable ALLOCATED.
*/
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
#define customMalloc malloc
#define customFree free
#endif

int calc_run() {
    char cmd[APP_MAXCMDLENGTH+1];
    int val, err;

    while (1) {
        fgets(cmd, APP_MAXCMDLENGTH, stdin);
        err = 0;
        val = calculate_value_str(cmd, &err);
        if (err != 0) {
            printf("Error %d\n", err);
        }
        else {
            printf("Value: %d\n", val);
        }
    }

    return 0;
}

expression* parse_cmd(char* str, int* err_out) {
    token* tokens = customMalloc(sizeof(token) * APP_MAXTOKENLENGTH);
    int token_len;
    if ((token_len = tokenize(str, tokens, APP_MAXCMDLENGTH)) < 0) {
        *err_out = -1;
        customFree(tokens);
        return NULL;
    }

    parse_state ps = { .tokens = tokens, .curpos = 0, .maxlength = token_len };
    expression* expr = parse_expression(&ps, err_out);
    // Error on extra tokens
    if (ps.curpos != ps.maxlength) {
        *err_out = -2;
        free_expression(expr);
    }

    customFree(tokens);
    return expr;
}

expression* parse_group(parse_state* ps, int* err_out) {
    if ((ps->tokens[ps->curpos]).type == TOK_PAREN && ps->tokens[ps->curpos].val == (int)'(') {
        ps->curpos++;
        int err = 0;
        expression* expr = parse_expression(ps, &err);
        if (err != 0) {
            *err_out = err;
            return NULL;
        }
        if (ps->tokens[ps->curpos].type == TOK_PAREN && ps->tokens[ps->curpos].val == (int)')') {
            ps->curpos++;
            return expr;
        }
        else {
            *err_out = -4;
            return NULL;
        }
    }
    else {
        *err_out = -3;
        return NULL;
    }
}

expression* parse_expression(parse_state* ps, int* err_out) {
    expression* term;
    int err = 0;

    term = parse_term(ps, &err);
    if (err != 0) {
        *err_out = err;
        return NULL;
    }
    while (ps->curpos < ps->maxlength) {
        if (ps->tokens[ps->curpos].type == TOK_OP && (ps->tokens[ps->curpos].val == (int)'+' || ps->tokens[ps->curpos].val == (int)'-')) {
            char op = (char) ps->tokens[ps->curpos].val;
            ps->curpos++;
            expression* term2 = parse_term(ps, &err);
            if (err != 0) {
                free_expression(term);
                *err_out = err;
                return NULL;
            }
            
            term = create_tree(op, term, term2);
        }
        else if (ps->tokens[ps->curpos].type == TOK_PAREN && ps->tokens[ps->curpos].val == (int)')') {
            return term;
        }
        else {
            *err_out = -1;
            free_expression(term);
            return NULL;
        }
    }
    return term;
}

expression* parse_term(parse_state* ps, int* err_out) {
    expression* expr;
    expression* expr2;
    int err = 0;

    expr = parse_group_or_number(ps, &err);
    if (err != 0) {
        *err_out = err;
        return NULL;
    }
    

    while (ps->curpos < ps->maxlength) {
        if (ps->tokens[ps->curpos].type == TOK_OP && (ps->tokens[ps->curpos].val == (int)'*' || ps->tokens[ps->curpos].val == (int)'/')) {
            char op = (char) ps->tokens[ps->curpos].val;
            ps->curpos++;
            expr2 = parse_group_or_number(ps, &err);
            if (err != 0) {
                free_expression(expr);
                *err_out = err;
                return NULL;
            }
            expr = create_tree(op, expr, expr2);
            expr2 = NULL;
        }
        else {
            break;
        }
    }

    return expr;
}

expression* parse_number(parse_state* ps, int* err_out) {
    if (ps->tokens[ps->curpos].type == TOK_NUMBER) {
        return create_number(ps->tokens[ps->curpos++].val);
    }
    else {
        *err_out = -1;
        return NULL;
    }
}

expression* parse_group_or_number(parse_state* ps, int* err_out) {
    expression* expr;
    int err = 0;
    if (ps->tokens[ps->curpos].type == TOK_PAREN) {
        expr = parse_group(ps, &err);
        if (err != 0) {
            *err_out = err;
            return NULL;
        }
        return expr;
    }
    else if (ps->tokens[ps->curpos].type == TOK_NUMBER) {
        expr = parse_number(ps, &err);
        if (err != 0) {
            *err_out = err;
            return NULL;
        }
        return expr;
    }
    else {
        *err_out = -1;
        return NULL;
    }
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
    if (expr == NULL) return 0;
    if (expr->type == EXPR_TREE) {
        free_expression((expr->data).tree.left);
        (expr->data).tree.left = NULL;
        free_expression((expr->data).tree.right);
        (expr->data).tree.right = NULL;
    }
    // free(expr);
    customFree(expr);
    return 0;
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
    int state = 0; //0 - single-char symbol; 1 - multi-char number
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
        // INVALID
        else {
            return -1;
        }

        if (len >= APP_MAXTOKENLENGTH) return -1;
    }

    return len;
}

int calculate_value(expression* expr, int* err_out) {
    if (expr->type == EXPR_NUMBER) {
        return expr->data.value;
    }
    else {
        int val1, val2;
        int err = 0;
        val1 = calculate_value(expr->data.tree.left, &err);
        if (err != 0) {
            *err_out = err;
            return 0;
        }
        val2 = calculate_value(expr->data.tree.right, &err);
        if (err != 0) {
            *err_out = err;
            return 0;
        }
        switch (expr->data.tree.op) {
            case '+':
                return val1 + val2;
            case '-':
                return val1 - val2;
            case '*':
                return val1 * val2;
            case '/':
                return val1 / val2;
            default:
                *err_out = -1;
                return 0;
        }
    }
}

int calculate_value_str(char* str, int* err_out) {
    int err = 0;
    expression* expr;

    expr = parse_cmd(str, &err);
    if (err != 0) {
        *err_out = err;
        return 0;
    }

    int val;
    val = calculate_value(expr, &err);
    free_expression(expr);

    if (err != 0) {
        *err_out = err;
        return 0;
    }

    return val;
}