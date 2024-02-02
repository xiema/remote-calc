#include <stddef.h>

extern int ALLOCATED;

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

/* Main parse function.
   Guarantees proper freeing of allocated memory upon failure.
*/
expression* parse_cmd(char* str, int* err_out);

int calculate_value(expression* expr, int* err_out);

int calculate_value_str(char* str, int* err_out);

/* Convert an expression into a human-readable string. Returns the length of the 
   resulting string on success. Returns a negative number upon error.
*/
int debug_expression(expression* expr, char* out, int curpos, int maxlength);

// Main function when running in 'calc' mode
int calc_run();

/* "Tokenize" a string into multi-digit numbers or single symbols.
   Returns the number of tokens on success, or a negative number on error.
*/
int tokenize(char* str, token* out, int maxlength);