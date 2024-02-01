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
}

void test_parsing() {
    expression* expr;
    char str[TEST_MAXDEBUGSTRLENGTH+1];
    char debugstr[TEST_MAXDEBUGSTRLENGTH+1];

    strcpy(str, "23 + 24");
    parse_state ps = { .str = str, .curpos = 0, .maxlength = 50 };
    TEST_ASSERT_EQUAL(0, parse_expression(ps, expr));
    debug_expression(expr, debugstr, 0, TEST_MAXDEBUGSTRLENGTH);
    TEST_ASSERT_EQUAL_STRING("(23+24)", debugstr);

    TEST_ASSERT_EQUAL(3, ALLOCATED);
    free_expression(expr);
    TEST_ASSERT_EQUAL(0, ALLOCATED);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_debug_expression);
    RUN_TEST(test_tokenization);

    UNITY_END();

    return 0;
}