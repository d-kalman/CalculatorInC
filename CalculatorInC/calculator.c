#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "calculator_util.h"
#include "stack.h"
#include "calculator.h"

int operator_precedence(char op) {
	switch (op) {
	case 'x':
	case '/':
		return 2;
	case '+':
	case '-':
		return 1;
	default:
		return 0;
	}
}

int operator_preceeds(char op1, char op2) {
	if (operator_precedence(op1) >= operator_precedence(op2))
		return 1;
	else
		return 0;
}

int parse(char* string, int size, Parseable** out_parseable, int* out_parsed_size, double previous_answer) {

	Parseable* parseables = (Parseable*)malloc(MAX_EXPRESSION_LENGTH * sizeof(Parseable));
	*out_parsed_size = 0;
	if (parseables == NULL) {
		*out_parseable = NULL;
		return MEMORY_ERROR;
	}

	Stack operator_stack;
	Stack* operator_stack_p = &operator_stack;

	initialize_stack(operator_stack_p);

	int end = 0;
	char number[MAX_NUMBER_LENGTH] = { '#' };
	int number_index = 0;
	//Discard beginning whitespace
	int begin = 0;
	while (string[begin] == ' ') {
		begin++;
	}
	//If the line reuses the previous answer, insert it as the first parseable
	if (allowed_followup_operation(string[begin])) {
		Parseable p = { .number = previous_answer, .operation = '#' };
		parseables[end] = p;
		end++;
	}

	for (int i = begin; i < size && end < MAX_EXPRESSION_LENGTH - 1; ++i) {
		//skip whitespace
		char current_char = string[i];
		if (current_char == ' ')
			continue;

		//decimal point or number: add to the number buffer to be parsed
		if (current_char == '.' || (current_char >= '0' && current_char <= '9')) {
			number[number_index] = current_char;
			number_index++;
			if (number_index >= MAX_NUMBER_LENGTH) {
				free(parseables);
				*out_parseable = NULL;
				return INPUT_ERROR;
			}
		}

		//Bracket or operator: process number buffer, if not empty, add to the parseable list
		//decide on the operator
		else {
			//Close the buffer, convert the number, reset buffer
			if (number_index > 0)
			{
				number[number_index] = '\0';
				double num = atof(number);
				number_index = 0;
				//Add number to the output
				Parseable p = { .number = num, .operation = '#' };
				parseables[end] = p;
				end++;
			}
			//Logic on operator:
			//Take the next operator
			//If the stack top precedence is higher push top to output
			//If same due to left right precedence push top to output
			Parseable current_operator = { .number = 0.0, .operation = current_char };

			//Push opening on operator stack
			if (current_operator.operation == '(')
			{
				push_to_stack(operator_stack_p, current_operator);
			}
			//If closing bracket: pop operators until opening found
			else if (current_operator.operation == ')')
			{
				while (!is_empty(operator_stack_p) && inspect(operator_stack_p).operation != '(') {
					parseables[end] = pop_stack(operator_stack_p);
					end++;
				}
				//If no matching bracket found
				//If this is true, that means that the loop was short circuited, and we couldn't find matching opener
				if (is_empty(operator_stack_p)) {
					free(parseables);
					*out_parseable = NULL;
					return PARSING_ERROR;
				}
				//if found: remove the opening bracket
				else
					pop_stack(operator_stack_p);

			}
			//Any other operator
			else if (allowed_input_operation(current_operator.operation)) {
				while (!is_empty(operator_stack_p) &&
					operator_preceeds(inspect(operator_stack_p).operation, current_operator.operation)) {
					parseables[end] = pop_stack(operator_stack_p);
					end++;
				}
				push_to_stack(operator_stack_p, current_operator);
			}
			//Operation not allowed
			else {
				free(parseables);
				*out_parseable = NULL;
				return INPUT_ERROR;
			}

		}
	}
	//If number left in buffer, close and insert it
	if (number_index > 0)
	{
		number[number_index] = '\0';
		double num = atof(number);
		number_index = 0;
		//Add number to the output
		Parseable p = { .number = num, .operation = '#' };
		parseables[end] = p;
		end++;
	}
	//Empty out the operator stack too
	while (!is_empty(operator_stack_p)) {
		//Unmatched bracket, error
		if (inspect(operator_stack_p).operation == '(') {
			free(parseables);
			*out_parseable = NULL;
			return PARSING_ERROR;
		}

		parseables[end] = pop_stack(operator_stack_p);
		end++;
	}

	//Add a close, so the resolver knows where to end
	parseables[end].operation = '!';
	*out_parseable = parseables;
	*out_parsed_size = end + 1;
	//for (int i = 0; i <= end; ++i)
	//	print_parseable(parseables[i]);
	return 0;
}

double process_line(char* string, int size, double answer)
{
	Parseable* parseable_list;
	int list_size = 0;
	int error_code = parse(string, size, &parseable_list, &list_size, answer);

	if (error_code == NO_ERROR) {
		double current_answer = resolve(parseable_list);
		free(parseable_list);
		return current_answer;
	}
	else if (error_code == MEMORY_ERROR) {
		printf("Memory allocation error\n");
	}
	else if (error_code == PARSING_ERROR) {
		printf("Parsing error\n");
	}
	else if (error_code == INPUT_ERROR) {
		printf("Input error\n");
	}
	if (error_code && parseable_list != NULL) {
		free(parseable_list);
	}
	return 0.0;

}

double resolve_operation(double lo, double ro, char operation)
{
	switch (operation) {
	case '+':
		return lo + ro;
	case '-':
		return lo - ro;
	case 'x':
		return lo * ro;
	case '/':
		return lo / ro;
	default:
		return 0.0;
	}
}

double resolve(Parseable* parseables) {
	double answer = 0;
	int index = 0;
	Stack stack;
	Stack* stack_p = &stack;
	initialize_stack(stack_p);
	while (parseables[index].operation != '!') {
		Parseable p = parseables[index];
		if (parseables[index].operation == '#') {
			push_to_stack(stack_p, p);
		}
		else {
			double ro = pop_stack(stack_p).number;
			double lo = pop_stack(stack_p).number;
			Parseable result = { .number = resolve_operation(lo, ro, p.operation), .operation = '#' };
			push_to_stack(stack_p, result);
		}
		index++;
	}
	answer = inspect(stack_p).number;
	return answer;
}

int allowed_input_operation(char c)
{
	int allowed = 0;
	allowed = allowed_followup_operation(c);
	switch (c) {
	case '(':
	case ')':
		allowed++;
		break;
	default:
		break;
	}
	return allowed;
}
int allowed_followup_operation(char c) {
	switch (c) {
	case 'x':
	case '+':
	case '/':
	case '-':
		return 1;
	default:
		return 0;
	}
}

int parsing_tests()
{
	int test_result = 0;

	Parseable* parsed_list = NULL;
	Parseable expected_result1[] = {
		{.number = 1.0, .operation = '#'},
		{.number = 2.0, .operation = '#' },
		{.number = 3.0, .operation = '#' },
		{.number = 0.0, .operation = 'x' },
		{.number = 0.0, .operation = '+'},
		{.number = 4.0, .operation = '#'},
		{.number = 0.0, .operation = '-'},
		{.number = 0.0, .operation = '!' }
	};
	int parsed_size = 0;
	printf("Parsing tests\n");
	char test[] = "1.0 + 2.00 x 3 - 4.0"; // 1 2 3 x + 4 - !
	int error_code = parse(test, strlen(test), &parsed_list, &parsed_size, 0);
	if (!compare_parseable_list(parsed_list, parsed_size, expected_result1, 8)) {
		test_result++;
	}
	free(parsed_list);
	parsed_list = NULL;


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
	char test2[] = "(1.0 + 2.00) x 3 - 4.0"; // 1 2 + 3 x 4 - !
	error_code = parse(test2, strlen(test2), &parsed_list, &parsed_size, 0);
	if (!compare_parseable_list(parsed_list, parsed_size, expected_result2, 8)) {
		test_result++;
		printf("Test 2 failed\n");
	}
	free(parsed_list);
	parsed_list = NULL;
	printf("\n----------------\n");
	return test_result;
}

int resolve_tests()
{
	printf("Resolve tests\n");
	int result = 0;
	Parseable* parsed_list;
	int parsed_size = 0;
	char test1[] = "1.0 + 2.00";
	int error_code = parse(test1, strlen(test1), &parsed_list, &parsed_size, 0);
	double answer = resolve(parsed_list); //3
	if (!float_compare(answer, 3.0)) {
		result++;
		printf("test 1 failed\n");
	};
	free(parsed_list);
	parsed_list = NULL;

	char test2[] = "1.5+2.11x3.2-4.1";
	error_code = parse(test2, strlen(test2), &parsed_list, &parsed_size, 0);
	answer = resolve(parsed_list); //4.152
	if (!float_compare(answer, 4.152)) {
		result++;
		printf("test 2 failed\n");
	}

	free(parsed_list);
	printf("\n----------------\n");
	return result;
}

void operator_tests() {
	printf("Operator tests\n");
	assert(operator_preceeds('x', '/') == 1);
	assert(operator_preceeds('-', 'x') == 0);
	assert(operator_preceeds('+', '/') == 0);
	assert(operator_preceeds('x', '+') == 1);
	printf("\n--------------\n");
}
