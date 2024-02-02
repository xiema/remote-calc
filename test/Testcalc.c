#include "unity.h"

#include "calc.h"
#include "socket.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TEST_MAXDEBUGSTRLENGTH 2000

void setUp() {

}

void tearDown() {

}

void test_debug_expression() {
    expression* expr;
    char debugstr[TEST_MAXDEBUGSTRLENGTH+1];

    // 11 + 12
    expr = create_tree(
        '+',
        create_number(11),
        create_number(12)
    );
    TEST_ASSERT_GREATER_OR_EQUAL(0, debug_expression(expr, debugstr, 0, TEST_MAXDEBUGSTRLENGTH));
    TEST_ASSERT_EQUAL_STRING("(11+12)", debugstr);

    // Should return val<0 if resulting string would be too long
    for (int maxlength = 1; maxlength < 7; maxlength++) {
        TEST_ASSERT_LESS_THAN(0, debug_expression(expr, debugstr, 0, maxlength));
    }
    
    // Ensure memory is properly allocated and deallocated
    TEST_ASSERT_EQUAL(3, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // (12 * (7 / 11)) + (421 - 2)
    expr = create_tree(
        '+',
        create_tree('*', create_number(12), create_tree('/', create_number(7), create_number(11))),
        create_tree('-', create_number(421), create_number(2))
    );
    TEST_ASSERT_GREATER_OR_EQUAL(0, debug_expression(expr, debugstr, 0, TEST_MAXDEBUGSTRLENGTH));
    TEST_ASSERT_EQUAL_STRING("((12*(7/11))+(421-2))", debugstr);

    // Ensure memory is properly allocated and deallocated
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);
}

void test_tokenization() {
    token* tokens = malloc(sizeof(token) * APP_MAXTOKENLENGTH);
    int length;
    
    int vals[] = { 1, (int)'+', 2, (int)'-', (int)'(', 3, (int)'*', 44, (int)')', (int)'/', 1234567890 };
    int types[] = { TOK_NUMBER, TOK_OP, TOK_NUMBER, TOK_OP, TOK_PAREN, TOK_NUMBER, TOK_OP, TOK_NUMBER, TOK_PAREN, TOK_OP, TOK_NUMBER };
    length = tokenize("  1 + 2 - ( 3 * 44  ) / 1234567890", tokens, 50);
    TEST_ASSERT_EQUAL(11, length);
    for (int i = 0; i < 11; i++) {
        TEST_ASSERT_EQUAL(vals[i], tokens[i].val);
        TEST_ASSERT_EQUAL(types[i], tokens[i].type);
    }

    free(tokens);
}

void debug_expression_with_null(expression* expr, char* str) {
    int len = debug_expression(expr, str, 0, TEST_MAXDEBUGSTRLENGTH);
    str[len] = '\0';
}

void test_parsing() {
    expression* expr;
    int err = 0;
    char debugstr[TEST_MAXDEBUGSTRLENGTH+1];

    // add/sub only
    expr = parse_cmd("23 + 24 - 1 + 321", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("(((23+24)-1)+321)", debugstr);
    TEST_ASSERT_EQUAL(7, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // all ops 1
    expr = parse_cmd("23 * 4 + 2 * 3", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("((23*4)+(2*3))", debugstr);
    TEST_ASSERT_EQUAL(7, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // all ops 2
    expr = parse_cmd("23 + 4 * 2 + 3", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("((23+(4*2))+3)", debugstr);
    TEST_ASSERT_EQUAL(7, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    expr = parse_cmd("(23 + 4)", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("(23+4)", debugstr);
    TEST_ASSERT_EQUAL(3, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // add/sub only with parens
    expr = parse_cmd("(23 + 4) - (2 + 3)", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("((23+4)-(2+3))", debugstr);
    TEST_ASSERT_EQUAL(7, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // parens
    expr = parse_cmd("(23 + 4) * (2 + 3)", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("((23+4)*(2+3))", debugstr);
    TEST_ASSERT_EQUAL(7, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // nested parens 1
    expr = parse_cmd("(1 + (2 - 3 * (5 / 2)))", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("(1+(2-(3*(5/2))))", debugstr);
    TEST_ASSERT_EQUAL(9, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // nested parens 2
    expr = parse_cmd("(2 + 4) * (99 - 2) / (3 - 1) + 24", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("((((2+4)*(99-2))/(3-1))+24)", debugstr);
    TEST_ASSERT_EQUAL(13, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // single values in parens
    expr = parse_cmd("(23) * (24)", &err);
    TEST_ASSERT_EQUAL(0, err);
    debug_expression_with_null(expr, debugstr);
    TEST_ASSERT_EQUAL_STRING("(23*24)", debugstr);
    TEST_ASSERT_EQUAL(3, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);
}

void test_parsing_error() {
    expression* expr;
    int err = 0;
    char debugstr[TEST_MAXDEBUGSTRLENGTH+1];

    expr = parse_cmd("- 24", &err);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    expr = parse_cmd("23 + - 24", &err);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    expr = parse_cmd("23 + 24 1", &err);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    expr = parse_cmd("23 1 + 24", &err);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    expr = parse_cmd("(23 + 1))", &err);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    expr = parse_cmd(")23 + 1", &err);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    expr = parse_cmd("()23 * 2", &err);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, ALLOCATED);
}

void test_calculation() {
    int val;
    int err = 0;

    val = calculate_value_str("1 + 1", &err);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(2, val);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    val = calculate_value_str("(1 + 2) * (5 - 3) / (1)", &err);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(6, val);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    val = calculate_value_str("1 + 2 + 3 + 4", &err);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(10, val);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    val = calculate_value_str("1 * 2 * 3 * 4", &err);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(24, val);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    val = calculate_value_str("10 - 3 - 6 - 1", &err);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, val);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    val = calculate_value_str("10 - (3 - 6) - 1", &err);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(12, val);
    TEST_ASSERT_EQUAL(0, ALLOCATED);

    // integer division
    val = calculate_value_str("4 / 3", &err);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(1, val);
    TEST_ASSERT_EQUAL(0, ALLOCATED);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_debug_expression);
    RUN_TEST(test_tokenization);
    RUN_TEST(test_parsing);
    RUN_TEST(test_parsing_error);
    RUN_TEST(test_calculation);

    UNITY_END();

    return 0;
}