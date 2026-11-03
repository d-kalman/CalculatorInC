#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "calculator_util.h"
#include "stack.h"
#include "calculator.h"

static int allowed_input_operation(char c)
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
static int allowed_followup_operation(char c) {
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

char operator_associativity(char op) {
	char associativity = 'L';
	if (op == '^')
		associativity = 'R';
	return associativity;
}

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
	if (associativity == 'R') {
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


static int add_parseable(ParserState* state, Parseable p) {
	int error_code = 0;
	if (state->end >= state->capacity - 1) {
		error_code = MEMORY_ERROR;
	}
	else{
		state->parseables[state->end++] = p;
	}
	return error_code;
}

static int drain_operator_stack(ParserState* state) {
	int error_code = NO_ERROR;
	while (!is_empty(state->operator_stack_p)) {
		//Unmatched bracket, error
		if (inspect(state->operator_stack_p).operation == '(') {
			error_code = PARSING_ERROR;
			break;
		}
		else {
			error_code = add_parseable(state, pop_stack(state->operator_stack_p));
			if (error_code)
				break;
		}
	}
	return error_code;
}
static int parse_arithmetic_operator(ParserState* state, Parseable p) {
	int error_code = NO_ERROR;
	while (!is_empty(state->operator_stack_p) &&
		operator_precedes(inspect(state->operator_stack_p).operation,
			p.operation,
			operator_associativity(p.operation)))
	{
		error_code = add_parseable(state, pop_stack(state->operator_stack_p));
		if (error_code)
			return error_code;
	}
	if (push_to_stack(state->operator_stack_p, p))
		error_code = MEMORY_ERROR;
	state->operator_expected = 0;
	return error_code;
}

static int parse_unary(ParserState* state, Parseable p) {
	int error_code = NO_ERROR;
	if (p.operation == '-') {
		if (state->operator_expected) {
			error_code = parse_arithmetic_operator(state, p);
		}
		//Unary operator
		else {
			p.operation = '~';
			if (push_to_stack(state->operator_stack_p, p))
				error_code = MEMORY_ERROR;
		}
	}
				
	else if (p.operation == '+') {
		//Unary + operator: skip
		//TODO: Decide if we want to do something with this.
		if(state->operator_expected)
			error_code = parse_arithmetic_operator(state, p);
	}
	state->operator_expected = 0;
	return error_code;
}

static int parse_close_paren(ParserState* state) {
	int error_code = NO_ERROR;
	if (state->operator_expected == 0)
		return PARSING_ERROR;
	while (!is_empty(state->operator_stack_p) && inspect(state->operator_stack_p).operation != '(') {
		error_code = add_parseable(state, pop_stack(state->operator_stack_p));
		if (error_code)
			return error_code;
	}
	//If no matching bracket found
	//If this is true, that means that the loop was short circuited, and we couldn't find matching opener
	if (is_empty(state->operator_stack_p)) {
		error_code = PARSING_ERROR;
	}
	//if found: remove the opening bracket
	else
		pop_stack(state->operator_stack_p);

	state->operator_expected = 1;
	return error_code;
}
static int parse_operator(ParserState* state, Parseable p) {
	int error_code = NO_ERROR;
	if (!allowed_input_operation(p.operation)) {
		error_code = INPUT_ERROR;
	}
	else {
		switch (p.operation) {
		case '-':
		case '+':
			error_code = parse_unary(state, p);
			break;
		case '(':
			if (state->operator_expected == 0) {
				if (push_to_stack(state->operator_stack_p, p)) {
					error_code = MEMORY_ERROR;
				} else {
					state->operator_expected = 0;
				}
			}
			else {
				error_code = PARSING_ERROR;
			}
			break;
		case 'x':
		case '/':
		case '^':
			error_code = parse_arithmetic_operator(state, p);
			break;
		case ')':
			error_code = parse_close_paren(state);
			break;

		}
	}
	return error_code;
}

static int parse_number(Input* input, ParserState* state) {
	int number_index = 0;
	char number[MAX_NUMBER_LENGTH+1] = { '#' };
	int error_code = NO_ERROR;
	if (state->operator_expected == 1) {
		error_code = PARSING_ERROR;
		return error_code;
	}
	int point_found = 0;
	while (input->index < input->size &&
		(input->line[input->index] == '.' || (input->line[input->index] >= '0' && input->line[input->index] <= '9')))
		{
		if (number_index >= MAX_NUMBER_LENGTH) {
			error_code = INPUT_ERROR;
			return error_code;
		}
		if (input->line[input->index] == '.') {
			if (point_found == 1)
				return INPUT_ERROR;
			else
				point_found = 1;
		}
		number[number_index] = input->line[input->index];
		number_index++;
		input->index++;
	}
	input->index--;
	if (number_index > 0)
	{
		number[number_index] = '\0';
		char* endptr = NULL;
		double num = strtod(number, &endptr);
		number_index = 0;
		//Add number to the output
		Parseable p = { .number = num, .operation = '#' };
		error_code = add_parseable(state, p);
	}
	state->operator_expected = 1;
	return error_code;
}

static double resolve_operation(double lo, double ro, char operation)
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

static int validate_logic(double lo, double ro, char operation) {
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

int parse(const char* l_string, size_t l_size, Parseable** out_parseable, int* out_parsed_size, AnswerState answer_state) {

	Input input = { .line = l_string, .size = l_size, .capacity = l_size + 2, .index = 0};
	Parseable* parseables = (Parseable*)malloc(input.capacity * sizeof(Parseable));
	if (parseables == NULL) {
		*out_parseable = NULL;
		*out_parsed_size = 0;
		return MEMORY_ERROR;
	}

	Stack operator_stack;
	Stack* operator_stack_p = &operator_stack;
	initialize_stack(operator_stack_p);


	ParserState parser_state = { .parseables = parseables,
								.end = 0, .capacity = input.capacity,
								.operator_stack_p = operator_stack_p,
								.operator_expected = 0};
	ParserState* parser_state_p = &parser_state;
	*out_parsed_size = 0;
	int error_code = NO_ERROR;
	//An operator needs both a left and a right operand.
	//int operator_expected = 0;
	//Discard leading whitespace
	int begin = 0;
	while (begin < input.size && input.line[begin] == ' ') {
		begin++;
	}
	if (begin >= input.size) {
		error_code = PARSING_ERROR;
		goto cleanup_on_error;
	}
		
	//If the line reuses the previous answer, insert it as the first parseable
	if (allowed_followup_operation(input.line[begin]) && answer_state.has_previous_answer) {
		Parseable p = { .number = answer_state.previous_answer, .operation = '#' };
		error_code = add_parseable(parser_state_p, p);
		if (error_code != 0)
			goto cleanup_on_error;
		parser_state.operator_expected = 1;
	}

	for (input.index = begin; input.index < input.size; ++input.index) {
		//skip whitespace
		char current_char = input.line[input.index];
		if (current_char == ' ')
			continue;

		if ((current_char >= '0' && current_char <= '9')) {
			error_code = parse_number(&input, parser_state_p);
			if (error_code != 0)
				goto cleanup_on_error;
		} else {
			Parseable current_operator = { .number = 0.0, .operation = current_char };
			error_code = parse_operator(parser_state_p, current_operator);
			if (error_code != 0)
				goto cleanup_on_error;
		}
	}


	//State left by trailing operator
	if (parser_state.operator_expected == 0) {
		error_code = PARSING_ERROR;
		goto cleanup_on_error;
	}

	//Empty out the operator stack too
	error_code = drain_operator_stack(parser_state_p);
	if (error_code != 0)
		goto cleanup_on_error;

	//Add a close, so the resolver knows where to end
	Parseable p = { .number = 0.0, .operation = '!' };
	parser_state.parseables[parser_state.end] = p;
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
	AnswerState ans_state = { .has_previous_answer = *has_previous_answer, .previous_answer = *answer };
	int error_code = parse(string, size, &parseable_list, &list_size, ans_state);

	if (error_code == NO_ERROR) {
		error_code = resolve(parseable_list, answer);
		if (error_code == NO_ERROR) {
			*has_previous_answer = 1;
		}
	}
	free(parseable_list);
	parseable_list = NULL;
	return error_code;
}

int resolve(Parseable* parseables, double* out_answer) {
	int index = 0;
	int error_code = NO_ERROR;
	Stack stack;
	Stack* stack_p = &stack;
	initialize_stack(stack_p);
	while (parseables[index].operation != '!') {
		Parseable p = parseables[index];
		if (parseables[index].operation == '#') {
			if (push_to_stack(stack_p, p)) {
				error_code = MEMORY_ERROR;
				break;
			}
		}
		else {
			double ro = 0;
			double lo = 0;
			ro = pop_stack(stack_p).number;
			if (p.operation != '~') {
				lo = pop_stack(stack_p).number;
			}
			if (!validate_logic(lo, ro, p.operation)) {
				error_code = LOGICAL_ERROR;
				break;
			}
			Parseable result = { .number = resolve_operation(lo, ro, p.operation), .operation = '#' };
			push_to_stack(stack_p, result);
		}
		index++;
	}
	if (error_code != NO_ERROR)
		return error_code;
	if (!is_empty(stack_p)) {
		*out_answer = inspect(stack_p).number;
	}
	else {
		error_code = PARSING_ERROR;
	}
	return error_code;
}
