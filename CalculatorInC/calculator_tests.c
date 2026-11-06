#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "calculator_tests.h"
#include "calculator_util.h"
#include "calculator.h"

void run_calculator_test_suite()
{
	run_operator_test_suite();
	run_parsing_test_suite();
	run_resolve_test_suite();
}

void run_operator_test_suite()
{
	printf("Operator tests\n");
	int tests_run = 4;
	int tests_passed = 0;
	tests_passed += run_operator_test('*', '/', 1);
	tests_passed += run_operator_test('-', '*', 0);
	tests_passed += run_operator_test('+', '/', 0);
	tests_passed += run_operator_test('*', '+',1);
	printf("\n--------------\n");
}
int run_operator_test(char op1, char op2, int expected_precedence)
{

	if (operator_precedes(op1, op2, operator_associativity(op2)) == expected_precedence)
		return 1;
	else
		return 0;
}
void run_parsing_test_suite()
{
	printf("Parsing tests\n");
	int tests_passed = 0;
	int tests_run = 0;
	int expected_result_length = 0;
	Token expected_result[] = {
		{.number = 1.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 2.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 3.0, .operation = '#', .type = TOK_NUMBER },
		{.number = 0.0, .operation = '*', .type = TOK_OPERATOR },
		{.number = 0.0, .operation = '+', .type = TOK_OPERATOR},
		{.number = 4.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0, .operation = '-', .type = TOK_OPERATOR},
		//{.number = 0.0, .operation = '!' }
	};
	TokenList expected_result_list;
	expected_result_list.tokens = expected_result;
	expected_result_list.capacity = sizeof(expected_result) / sizeof(Token);
	expected_result_list.end = sizeof(expected_result) / sizeof(Token);
	tests_passed += run_parsing_test("1.0 + 2.00 * 3 - 4.0", &expected_result_list, NO_ERROR, 0.0, 0, __LINE__); // 1 2 3 * + 4 -
	tests_run++;

	Token expected_result2[] = {
		{.number = 1.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 2.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0, .operation = '+', .type = TOK_OPERATOR},
		{.number = 3.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0, .operation = '*', .type = TOK_OPERATOR},
		{.number = 4.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0, .operation = '-', .type = TOK_OPERATOR},
	};
	expected_result_list.tokens = expected_result2;
	expected_result_list.capacity = sizeof(expected_result2) / sizeof(Token);
	expected_result_list.end = sizeof(expected_result2) / sizeof(Token);
	tests_passed += run_parsing_test("(1.0 + 2.00) * 3 - 4.0", &expected_result_list, NO_ERROR, 0.0, 0, __LINE__); // 1 2 + 3 * 4 -
	tests_run++;

	Token expected_result3[] = {
		{.number = 5.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0, .operation = '~', .type = TOK_OPERATOR},
		{.number = 5.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0, .operation = '*', .type = TOK_OPERATOR},
	};
	expected_result_list.tokens = expected_result3;
	expected_result_list.capacity = sizeof(expected_result3) / sizeof(Token);
	expected_result_list.end = sizeof(expected_result3) / sizeof(Token);
	tests_passed += run_parsing_test("-5*5", &expected_result_list, NO_ERROR, 0.0, 0, __LINE__); // 5 ~ 5 *
	tests_run++;

	Token expected_result4[] = {
		{.number = 5.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 3.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 4.0, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0, .operation = '*', .type = TOK_OPERATOR},
		{.number = 0.0, .operation = '-', .type = TOK_OPERATOR},
	};
	expected_result_list.tokens = expected_result4;
	expected_result_list.capacity = sizeof(expected_result4) / sizeof(Token);
	expected_result_list.end = sizeof(expected_result4) / sizeof(Token);
	tests_passed += run_parsing_test("-3*4", &expected_result_list, NO_ERROR, 5.0, 1, __LINE__); // 5 3 4 * - 
	tests_run++;

	Token expected_result5[] = {
		{.number = 0.0,  .operation = '#', .type = TOK_NUMBER},
		{.number = 3.11, .operation = '#', .type = TOK_NUMBER},
		{.number = 4.83, .operation = '#', .type = TOK_NUMBER},
		{.number = 0.0,  .operation = '*', .type = TOK_OPERATOR},
		{.number = 0.0,  .operation = '-', .type = TOK_OPERATOR},
	};
	expected_result_list.tokens = expected_result5;
	expected_result_list.capacity = sizeof(expected_result5) / sizeof(Token);
	expected_result_list.end = sizeof(expected_result5) / sizeof(Token);
	tests_passed += run_parsing_test("-3.11*4.83", &expected_result_list, NO_ERROR, 0.0, 1, __LINE__); // 5 3 4 * - 
	tests_run++;

	//End tests
	if (tests_passed == tests_run) {
		printf("Parsing tests passed: %d/%d passed\n", tests_passed, tests_run);
	}
	else {
		printf("Parsing tests failed: %d/%d passed\n", tests_passed, tests_run);
	}
}

int run_parsing_test(const char* expression, 
					TokenList* expected_result,
					int expected_error_code, 
					double previous_answer, 
					int has_previous_answer, int line)
{
	TokenList* token_list = NULL;
	int test_result = 0;
	AnswerState ans_state = { .has_previous_answer = has_previous_answer, .previous_answer = previous_answer };
	int error_code = parse(expression, strlen(expression), &token_list, ans_state);
	if (error_code != expected_error_code) {
		printf("Test of expression %s at line %d failed mismatched error code\n", expression, line);
	}
	else if (error_code != NO_ERROR) {
		test_result = 1;
	}
	else if (!compare_token_list(token_list, expected_result)) {
		printf("Test of expression %s at line %d failed mismatched parsed list\n", expression, line);
	}
	else {
		test_result = 1;
	}

	if (token_list != NULL) {
		free_token_list(token_list);
		token_list = NULL;
	}
	return test_result;
}

void run_resolve_test_suite()
{
	static const struct { const char* expression; double expected; int error_code; double previous_answer; int has_previous_answer; } cases[] = {
		//Basic arithmetics
		{ "1.0 + 2.00",       3.0,   NO_ERROR, 0, 0     },
		{ "1.5+2.11*3.2-4.1", 4.152, NO_ERROR, 0, 0     },
		{ "2+3*4",            14.0,  NO_ERROR, 0, 0     },
		{ "(2+3)*4",          20.0,  NO_ERROR, 0, 0     },
		{ "10/4",             2.5,   NO_ERROR, 0, 0     },
		//Unary operator and chaining
		{ "-5*3",            -15,    NO_ERROR, 0, 0     },
		{ "-3*4",            -7,     NO_ERROR, 5, 1     },
		{ "-3*-4",            17,    NO_ERROR, 5, 1     },
		{ "-3*-3",            9,     NO_ERROR, 0, 0     },
		{ "(-2)*(-2)",        4,     NO_ERROR, 0, 0     },
		{ "(-2)*8",          -16,    NO_ERROR, 0, 0     },
		{ "--(-3.451)",      -3.451, NO_ERROR, 0, 0     },
		{ "+-(+137.8651)",   -137.8651, NO_ERROR, 0, 0  },
		//Exponentiation
		{ "2^10",             1024, NO_ERROR, 0, 0  }, //basic
		{ "4^0.5",            2,     NO_ERROR, 0, 0  }, //root
		{ "2 ^ 3 ^ 2",            512,     NO_ERROR, 0, 0  }, //right associativity
		{ "(2 ^ 3) ^ 2",          64,     NO_ERROR, 0, 0  },
		{ "2 ^ 3 * 2",            16,     NO_ERROR, 0, 0  },
		{ "(-1) ^ 3",            -1,     NO_ERROR, 0, 0  },
		{ "2^-3^2", 0.001953125,     NO_ERROR, 0, 0 },
		//Logical errors
		{ "(-8) ^ (1 / 3)",       0,     LOGICAL_ERROR, 0, 0  }, //restricted domain
		{ "1/0",			      0,     LOGICAL_ERROR, 0, 0},   //division by zero
		{ "(-1)^0.5",			  0,     LOGICAL_ERROR, 0, 0 },  //sqrt of -1
		//Error paths
		{ "(2 +3", 0,     PARSING_ERROR, 0, 0 },
		{ "2+3)", 0,     PARSING_ERROR, 0, 0 },
		{ "2 3", 0,     PARSING_ERROR, 0, 0 },
		{ "2+",	0,		PARSING_ERROR, 0, 0 },
		{ "()", 0, PARSING_ERROR, 0, 0},
		{ "99999999999999999999 + 1", 1e20, NO_ERROR, 0, 0}, //number just fits 
		{ "999999999999999999999 + 1", 0, INPUT_ERROR, 0, 0}, //number too long issue
		{ "(2 + )", 0, PARSING_ERROR, 0, 0},
		{ "2*()", 0, PARSING_ERROR, 0, 0},
		{ "(-)", 0, PARSING_ERROR, 0, 0},		
	};
	int tests_passed = 0;
	int tests_run = (int)(sizeof(cases) / sizeof(cases[0]));
	for (int i = 0; i < tests_run; ++i) {
		tests_passed += run_resolve_test(cases[i].expression, 
										cases[i].expected, 
										cases[i].error_code, 
										cases[i].previous_answer, 
										cases[i].has_previous_answer, i + 1);
	}
	if (tests_passed == tests_run) {
		printf("Resolve tests passed: %d/%d passed\n", tests_passed, tests_run);
	}
	else {
		printf("Resolve tests failed: %d/%d passed\n", tests_passed, tests_run);
	}

}

int run_resolve_test(const char* expression, double expected_result,
					int expected_error_code, 
					double previous_answer, 
					int has_previous_answer, int id)
{
	TokenList* tokens = NULL;
	int test_result = 0;
	AnswerState ans_state = { .has_previous_answer = has_previous_answer, .previous_answer = previous_answer };
	ErrorCode error_code = parse(expression, (int)strlen(expression), &tokens, ans_state);
	if (error_code != NO_ERROR) {
		if (error_code == expected_error_code) {
			return 1;
		}
		else {
			printf("Test %d failed: Parsing of %s with %d\n", id, expression, error_code);
			return 0;
		}
	}
	double answer = 0.0;
	error_code = resolve(tokens, &answer);
	if (error_code == NO_ERROR) {
		if (!float_compare(answer, expected_result)) {
			printf("Test %d failed: %s %.10g not matching %.10g\n", id, expression, answer, expected_result);
		}
		else {
			test_result = 1;
		}
	}
	else if (error_code == expected_error_code){
		test_result = 1;
	}
	else {
		printf("Test %d failed: %s with EC %d \n", id, expression, error_code);
	}
	free_token_list(tokens);
	return test_result;
}
