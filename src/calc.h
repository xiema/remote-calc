#include <stddef.h>

typedef enum {
    EXPR_NUMBER,
    EXPR_TREE
} EXPRESSION_TYPE;

typedef struct expressiontree expressiontree;
typedef struct expression expression;

struct expressiontree {
    char op;
    expression* left;
    expression* right;
};

struct expression {
    EXPRESSION_TYPE type;
    union {
        int value;
        expressiontree tree;
    } data;
};

typedef enum {
    TOK_NUMBER,
    TOK_OP,
    TOK_PAREN
} TOK_TYPE;

typedef struct token {
    TOK_TYPE type;
    int val;
} token;

typedef struct parse_state {
    token* tokens;
    int curpos;
    int maxlength;
} parse_state;

expression* create_number(int value);

expression* create_tree(char op, expression* left, expression* right);

int free_expression(expression* expr);

expression* parse_cmd(char* str, int* err_out);

expression* parse_expression(parse_state* ps, int* err_out);

expression* parse_term(parse_state* ps, int* err_out);

expression* parse_number(parse_state* ps, int* err_out);

expression* parse_group(parse_state* ps, int* err_out);

expression* parse_group_or_number(parse_state* ps, int* err_out);

int calculate_value(expression* expr, int* err_out);

int calculate_value_str(char* str, int* err_out);

int debug_expression(expression* expr, char* out, int curpos, int maxlength);

int calc_run();

extern int ALLOCATED;

int tokenize(char* str, token* out, int maxlength);