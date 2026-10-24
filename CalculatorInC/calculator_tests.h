#pragma once
#include "parseable.h"

void run_calculator_test_suite();
void run_parsing_test_suite();
int run_parsing_test(const char* expression, 
					Parseable expected_result[], 
					int expected_result_length, 
					int expected_error_code, 
					double previous_answer, 
					int line);
void run_resolve_test_suite();
int run_resolve_test(const char* expression, const double expected_result, int expected_error_code, int id);
void run_operator_test_suite();
int run_operator_test(char op1, char op2, int expected_precedence);