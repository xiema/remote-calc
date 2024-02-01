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

typedef struct parse_state {
    char* str;
    int curpos;
    int maxlength;
} parse_state;

expression* create_number(int value);

expression* create_tree(char op, expression* left, expression* right);

int free_expression(expression* expr);

int parse_expression(parse_state ps, expression* out);

int debug_expression(expression* expr, char* out, int curpos, int maxlength);

int calc_run();

extern int ALLOCATED;