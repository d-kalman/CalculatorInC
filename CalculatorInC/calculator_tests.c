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
	tests_passed += run_operator_test('x', '/', 1);
	tests_passed += run_operator_test('-', 'x', 0);
	tests_passed += run_operator_test('+', '/', 0);
	tests_passed += run_operator_test('x', '+',1);
	printf("\n--------------\n");
}
int run_operator_test(char op1, char op2, int expected_precedence)
{

	if (operator_preceeds(op1, op2, operator_associativity(op2)) == expected_precedence)
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
	Parseable expected_result[] = {
		{.number = 1.0, .operation = '#'},
		{.number = 2.0, .operation = '#' },
		{.number = 3.0, .operation = '#' },
		{.number = 0.0, .operation = 'x' },
		{.number = 0.0, .operation = '+'},
		{.number = 4.0, .operation = '#'},
		{.number = 0.0, .operation = '-'},
		{.number = 0.0, .operation = '!' }
	};
	expected_result_length = sizeof(expected_result) / sizeof(Parseable);
	tests_passed += run_parsing_test("1.0 + 2.00 x 3 - 4.0", expected_result, expected_result_length, NO_ERROR, 0.0, 0, __LINE__); // 1 2 3 x + 4 -
	tests_run++;

	Parseable expected_result2[] = {
		{.number = 1.0, .operation = '#'},
		{.number = 2.0, .operation = '#' },
		{.number = 0.0, .operation = '+'},
		{.number = 3.0, .operation = '#' },
		{.number = 0.0, .operation = 'x' },
		{.number = 4.0, .operation = '#'},
		{.number = 0.0, .operation = '-'},
		{.number = 0.0, .operation = '!' }
	};
	expected_result_length = sizeof(expected_result2) / sizeof(Parseable);
	tests_passed += run_parsing_test("(1.0 + 2.00) x 3 - 4.0", expected_result2, expected_result_length, NO_ERROR, 0.0, 0, __LINE__); // 1 2 + 3 x 4 -
	tests_run++;

	Parseable expected_result3[] = {
		{.number = 5.0, .operation = '#'},
		{.number = 0.0, .operation = '~' },
		{.number = 5.0, .operation = '#'},
		{.number = 0.0, .operation = 'x'},
		{.number = 0.0, .operation = '!' }
	};
	expected_result_length = sizeof(expected_result3) / sizeof(Parseable);
	tests_passed += run_parsing_test("-5x5", expected_result3, expected_result_length, NO_ERROR, 0.0, 0, __LINE__); // 5 ~ 5 x
	tests_run++;

	Parseable expected_result4[] = {
		{.number = 5.0, .operation = '#'},
		{.number = 3.0, .operation = '#'},
		{.number = 4.0, .operation = '#'},
		{.number = 0.0, .operation = 'x'},
		{.number = 0.0, .operation = '-'},
		{.number = 0.0, .operation = '!' }
	};
	expected_result_length = sizeof(expected_result4) / sizeof(Parseable);
	tests_passed += run_parsing_test("-3x4", expected_result4, expected_result_length, NO_ERROR, 5.0, 1, __LINE__); // 5 3 4 x - 
	tests_run++;

	Parseable expected_result5[] = {
		{.number = 0.0, .operation = '#'},
		{.number = 3.11, .operation = '#'},
		{.number = 4.83, .operation = '#'},
		{.number = 0.0, .operation = 'x'},
		{.number = 0.0, .operation = '-'},
		{.number = 0.0, .operation = '!' }
	};
	expected_result_length = sizeof(expected_result5) / sizeof(Parseable);
	tests_passed += run_parsing_test("-3.11x4.83", expected_result5, expected_result_length, NO_ERROR, 0.0, 1, __LINE__); // 5 3 4 x - 
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
					Parseable expected_result[], 
					int expected_result_length, 
					int expected_error_code, 
					double previous_answer, 
					int has_previous_answer, int line)
{
	Parseable* parsed_list = NULL;
	int parsed_size = 0;
	int test_result = 0;
	int error_code = parse(expression, strlen(expression), &parsed_list, &parsed_size, previous_answer, has_previous_answer);
	if (error_code != expected_error_code) {
		printf("Test of expression %s at line %d failed mismatched error code\n", expression, line);
	}
	else if (!compare_parseable_list(parsed_list, parsed_size, expected_result, expected_result_length)) {
		printf("Test of expression %s at line %d failed mismatched parsed list\n", expression, line);
	}
	else {
		test_result = 1;
	}

	if (parsed_list != NULL) {
		free(parsed_list);
		parsed_list = NULL;
	}
	return test_result;
}

void run_resolve_test_suite()
{
	static const struct { const char* expression; double expected; int error_code; double previous_answer; int has_previous_answer; } cases[] = {
		//Basic arithmetics
		{ "1.0 + 2.00",       3.0,   NO_ERROR, 0, 0     },
		{ "1.5+2.11x3.2-4.1", 4.152, NO_ERROR, 0, 0     },
		{ "2+3x4",            14.0,  NO_ERROR, 0, 0     },
		{ "(2+3)x4",          20.0,  NO_ERROR, 0, 0     },
		{ "10/4",             2.5,   NO_ERROR, 0, 0     },
		//Unary operator and chaining
		{ "-5x3",            -15,    NO_ERROR, 0, 0     },
		{ "-3x4",            -7,     NO_ERROR, 5, 1     },
		{ "-3x-3",            9,     NO_ERROR, 0, 0     },
		{ "(-2)x(-2)",        4,     NO_ERROR, 0, 0     },
		{ "(-2)x8",          -16,    NO_ERROR, 0, 0     },
		{ "--(-3.451)",      -3.451, NO_ERROR, 0, 0     },
		{ "+-(+137.8651)",   -137.8651, NO_ERROR, 0, 0  },
		//Exponentiation
		{ "2^10",             1024, NO_ERROR, 0, 0  }, //basic
		{ "4^0.5",            2,     NO_ERROR, 0, 0  }, //root
		{ "2 ^ 3 ^ 2",            512,     NO_ERROR, 0, 0  }, //right associativity
		{ "(2 ^ 3) ^ 2",          64,     NO_ERROR, 0, 0  },
		{ "2 ^ 3 x 2",            16,     NO_ERROR, 0, 0  },
		//Logical errors
		{ "(-8) ^ (1 / 3)",       0,     LOGICAL_ERROR, 0, 0  }, // restricted domain
		{ "1/0",			  0,     LOGICAL_ERROR, 0, 0}, // division by zero
		{ "(-1)^0.5",			  0,     LOGICAL_ERROR, 0, 0 } //sqrt of -1
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
	Parseable* parseables = NULL;
	int size = 0;
	int test_result = 0;
	int error_code = parse(expression, (int)strlen(expression), &parseables, &size, previous_answer, has_previous_answer);
	if (error_code != NO_ERROR) {
		printf("Test %d failed: Parsing of %s with %d\n", id, expression, error_code);
		free(parseables);
		return 0;
	}
	double answer = 0.0;
	error_code = resolve(parseables, &answer);
	if (error_code == NO_ERROR) {
		if (!float_compare(answer, expected_result)) {
			printf("Test %d failed: %.10g not matching %.10g\n", id, answer, expected_result);
		}
		else {
			test_result = 1;
		}
	}
	else if (error_code == expected_error_code){
		test_result = 1;
	}
	else {
		printf("Test %d failed: EC %d\n", id, error_code);
	}
	free(parseables);
	parseables = NULL;
	return test_result;
}
