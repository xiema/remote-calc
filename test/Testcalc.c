#include "unity.h"

#include "calc.h"
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
    // RUN_TEST(test_parsing);

    UNITY_END();

    return 0;
}