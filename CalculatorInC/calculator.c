#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "calculator_util.h"
#include "stack.h"
#include "calculator.h"

int operator_precedence(char op) {
	switch (op) {
	case '^':
		return 4;
	case '~':
		return 3;
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

int operator_precedes(char op1, char op2, char associativity) {
	int preceeds = 0;
	if (associativity == 'R')
	{
		if (operator_precedence(op1) > operator_precedence(op2))
			preceeds = 1;
		else
			preceeds = 0;
	}
	else {
		if (operator_precedence(op1) >= operator_precedence(op2)) {
			preceeds = 1;
		}
		else {
			preceeds = 0;
		}
	}
	return preceeds;
}

char operator_associativity(char op) {
	char associativity = 'L';
	if (op == '^')
		associativity = 'R';
	return associativity;
}

int add_parseable(ParserState* state, Parseable p) {
	int error_code = 0;
	if (state->end >= state->capacity - 1) {
		error_code = PARSING_ERROR;
	}
	state->parseables[state->end++] = p;
	return error_code;
}

int parse_number(Input* input, ParserState* state) {
	int number_index = 0;
	char number[MAX_NUMBER_LENGTH] = { '#' };
	int error_code = NO_ERROR;
	if (state->operator_expected == 1) {
		error_code = PARSING_ERROR;
	}
	while (input->index < input->size &&
		(input->line[input->index] == '.' || (input->line[input->index] >= '0' && input->line[input->index] <= '9')))
		{
		number[number_index] = input->line[input->index];
		number_index++;
		if (number_index >= MAX_NUMBER_LENGTH) {
			error_code = INPUT_ERROR;
		}
		input->index++;
	}
	input->index--;
	if (number_index > 0)
	{
		number[number_index] = '\0';
		double num = atof(number);
		number_index = 0;
		//Add number to the output
		Parseable p = { .number = num, .operation = '#' };
		error_code = add_parseable(state, p);
	}
	state->operator_expected = 1;
	return error_code;
}

int parse(const char* l_string, size_t l_size, Parseable** out_parseable, int* out_parsed_size, double previous_answer, int has_previous_answer) {

	Parseable* parseables = (Parseable*)malloc(MAX_EXPRESSION_LENGTH * sizeof(Parseable));
	if (parseables == NULL) {
		*out_parseable = NULL;
		return MEMORY_ERROR;
	}

	Stack operator_stack;
	Stack* operator_stack_p = &operator_stack;
	initialize_stack(operator_stack_p);

	Input input = { .line = l_string, .size = l_size, .capacity = MAX_EXPRESSION_LENGTH, .index = 0};

	ParserState parser_state = { .parseables = parseables, 
								.end = 0, .capacity = MAX_EXPRESSION_LENGTH, 
								.operator_stack_p = operator_stack_p, 
								.operator_expected = 0};
	ParserState* parser_state_p = &parser_state;
	*out_parsed_size = 0;
	
	//int end = 0;
	int error_code = NO_ERROR;
	//An operator needs both a left and a right operand.
	//int operator_expected = 0;
	//Discard leading whitespace
	int begin = 0;
	while (input.line[begin] == ' ' && begin < input.size) {
		begin++;
	}
	//If the line reuses the previous answer, insert it as the first parseable
	if (allowed_followup_operation(input.line[begin]) && has_previous_answer) {
		Parseable p = { .number = previous_answer, .operation = '#' };
		error_code = add_parseable(parser_state_p, p);
		if (error_code != 0)
			goto cleanup_on_error;
		parser_state.operator_expected = 1;
	}

	for (input.index = begin; input.index < input.size && parser_state.end < MAX_EXPRESSION_LENGTH - 1; ++input.index) {
		//skip whitespace
		char current_char = input.line[input.index];
		if (current_char == ' ')
			continue;

		//New approach: after operator either number or opening bracket expected
		//after number or closing bracket operator is expected
		//Read numbers in one swoop
		if ((current_char >= '0' && current_char <= '9')) {
			if (parser_state.operator_expected == 1) {
				error_code = PARSING_ERROR;
				goto cleanup_on_error;
			}
			else {
				error_code = parse_number(&input, parser_state_p);
				if (error_code != 0)
					goto cleanup_on_error;
			}
		}
		//Bracket or operator:
		//decide on the operator
		//Logic on operator:
		//Take the next operator
		//If the stack top precedence is higher push top to output
		//If same due to left right precedence push top to output
		else {
			Parseable current_operator = { .number = 0.0, .operation = current_char };
			//If we find an operator when operand expected
			//It can be either a unary operator or a '('
			if (parser_state.operator_expected == 0) {
				//Push opening on operator stack
				if (current_char == '(') {
					push_to_stack(operator_stack_p, current_operator);
					parser_state.operator_expected = 0;
				}
				//Decide if chained subtract or unary operator
				else if (current_char == '-') {
					//Chained subtract
					if (has_previous_answer) {
						push_to_stack(operator_stack_p, current_operator);
						parser_state.operator_expected = 0;
					}
					//Unary operator
					else {
						current_operator.operation = '~';
						push_to_stack(operator_stack_p, current_operator);
						parser_state.operator_expected = 0;
					}
				}
				//Unary + operator: skip
				else if (current_char == '+') {
					continue;
				}
				else {
					error_code = PARSING_ERROR;
					goto cleanup_on_error;
				}
			}
			//If closing bracket: pop operators until opening found
			else if (current_operator.operation == ')')
			{
				while (!is_empty(operator_stack_p) && inspect(operator_stack_p).operation != '(') {
					error_code = add_parseable(parser_state_p, pop_stack(parser_state.operator_stack_p));
				}
				//If no matching bracket found
				//If this is true, that means that the loop was short circuited, and we couldn't find matching opener
				if (is_empty(operator_stack_p)) {
					error_code = PARSING_ERROR;
					goto cleanup_on_error;
				}
				//if found: remove the opening bracket
				else
					pop_stack(operator_stack_p);

				parser_state.operator_expected = 1;
			}
			//Any other operator
			else if (allowed_input_operation(current_operator.operation)) {
				while (!is_empty(operator_stack_p) &&
					operator_precedes(inspect(operator_stack_p).operation,
						current_operator.operation,
						operator_associativity(current_operator.operation)))
				{
					error_code = add_parseable(parser_state_p, pop_stack(operator_stack_p));
					if (error_code != 0)
						goto cleanup_on_error;
				}
				push_to_stack(operator_stack_p, current_operator);
				parser_state.operator_expected = 0;
			}
			//Operation not allowed
			else {
				error_code = INPUT_ERROR;
				goto cleanup_on_error;
			}
		}
	}
	//State left by trailing operator
	if (parser_state.operator_expected == 0) {
		error_code = PARSING_ERROR;
		goto cleanup_on_error;

	}

	//Empty out the operator stack too
	while (!is_empty(operator_stack_p)) {
		//Unmatched bracket, error
		if (inspect(operator_stack_p).operation == '(') {
			error_code = PARSING_ERROR;
			goto cleanup_on_error;
		}
		error_code = add_parseable(parser_state_p, pop_stack(operator_stack_p));
		if (error_code != 0)
			goto cleanup_on_error;
	}

	//Add a close, so the resolver knows where to end
	parser_state.parseables[parser_state.end].operation = '!';
	*out_parseable = parser_state.parseables;
	*out_parsed_size = parser_state.end + 1;
	return NO_ERROR;

cleanup_on_error:
	free(parseables);
	*out_parseable = NULL;
	return error_code;
}

//Changes answer only on success
int process_line(const char* string, int size, double* answer, int* has_previous_answer)
{
	Parseable* parseable_list = NULL;
	int list_size = 0;

	int error_code = parse(string, size, &parseable_list, &list_size, *answer, *has_previous_answer);

	if (error_code == NO_ERROR) {
		error_code = resolve(parseable_list, answer);
		if (error_code == NO_ERROR) {
			*has_previous_answer = 1;
		}
		free(parseable_list);
	}
	else if (error_code && parseable_list != NULL) {
		free(parseable_list);
	}
	return error_code;
}

double resolve_operation(double lo, double ro, char operation)
{
	switch (operation) {
	case '^':
		return pow(lo, ro);
	case '~':
		return -1 * ro;
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

int resolve(Parseable* parseables, double* out_answer) {
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
			double ro = 0;
			double lo = 0;
			ro = pop_stack(stack_p).number;
			if (p.operation != '~') {
				lo = pop_stack(stack_p).number;
			}
			if (!validate_logic(lo, ro, p.operation)) {
				return LOGICAL_ERROR;
			}
			Parseable result = { .number = resolve_operation(lo, ro, p.operation), .operation = '#' };
			push_to_stack(stack_p, result);
		}
		index++;
	}
	if (!is_empty(stack_p)) {
		*out_answer = inspect(stack_p).number;
		return NO_ERROR;
	}
	else
	{
		return PARSING_ERROR;
	}
}

int allowed_input_operation(char c)
{
	int allowed = 0;
	allowed = allowed_followup_operation(c);
	switch (c) {
	case '(':
	case ')':
	case '^':
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
int validate_logic(double lo, double ro, char operation) {
	//Division by 0
	if (ro == 0 && operation == '/')
		return 0;
	//Non integer root of a negative number is not allowed
	else if (operation == '^' && lo < 0.0) {
		// Check if exponent is integer
		double intpart;
		if (modf(fabs(ro), &intpart) != 0.0)
			return 0;
		else
			return 1;
	}
	else {
		return 1;
	}
}