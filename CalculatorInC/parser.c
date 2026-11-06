#include <stdlib.h>
#include <string.h>
#include "parser.h"

static FunctionId get_function_id(char* func_name) {
	if (strcmp(func_name, "sin") == 0)
		return SIN;
	else if (strcmp(func_name, "cos") == 0)
		return COS;
	else if (strcmp(func_name, "abs") == 0)
		return ABS;
	else
		return NOT_FUNCTION;
}

static int allowed_followup_operation(char c) {
	switch (c) {
	case '*':
	case '/':
	case '+':
	case '-':
		return 1;
	default:
		return 0;
	}
}

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
ErrorCode add_previous_answer(Input* input, ParserState* state, AnswerState answer_state) {
	ErrorCode error_code = NO_ERROR;
	//If the line reuses the previous answer, insert it as the first parseable
	if (allowed_followup_operation(input->line[input->index]) && answer_state.has_previous_answer) {
		Token t = create_num_token(answer_state.previous_answer);
		error_code = add_token(state->parsed_tokens, t);
	}
	return error_code;
}

static ErrorCode parse_number(Input* input, ParserState* state) {
	int number_index = 0;
	char number[MAX_NUMBER_LENGTH + 1] = { '#' };
	ErrorCode error_code = NO_ERROR;
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
		////Add number to the parsed tokens
		Token t = create_num_token(num);
		error_code = add_token(state->parsed_tokens, t);
	}
	return error_code;
}

static ErrorCode parse_function_token(Input* input, ParserState* state) {
	ErrorCode error_code = NO_ERROR;
	int name_index = 0;
	char name[MAX_NUMBER_LENGTH + 1] = { '#' };

	while (input->index < input->size && input->line[input->index] != '(')
	{
		if (name_index >= MAX_NUMBER_LENGTH) {
			error_code = INPUT_ERROR;
			return error_code;
		}
		name[name_index] = input->line[input->index];
		name_index++;
		input->index++;
	}
	if (input->line[input->index] != '(') {
		error_code = INPUT_ERROR;
		return error_code;
	}
	input->index--;
	if (name_index > 0)
	{
		name[name_index] = '\0';
		char* endptr = NULL;

		Token t = create_func_token(get_function_id(name));
		if (t.function_id == NOT_FUNCTION)
			error_code = PARSING_ERROR;
		else
			error_code = add_token(state->parsed_tokens, t);
	}
	return error_code;
}

ErrorCode tokenize(Input* input, ParserState* state) {
	ErrorCode error_code = NO_ERROR;
	for (; input->index < input->size; ++(input->index)) {
		//skip whitespace
		char current_char = input->line[input->index];
		if (current_char == ' ')
			continue;

		if ((current_char >= '0' && current_char <= '9')) {
			error_code = parse_number(input, state);
			if (error_code != 0)
				return error_code;
		}
		else if (current_char >= 'a' && current_char <= 'z') {
			error_code = parse_function_token(input, state);
		}

		else if (current_char == '(' || current_char == ')') {
			Token t = create_op_token(current_char);
			error_code = add_token(state->parsed_tokens, t);
		}
		else {
			Token t = create_op_token(current_char);
			if (allowed_input_operation(t.operation)) {
				error_code = add_token(state->parsed_tokens, t);
			}
			else {
				error_code = INPUT_ERROR;
			}
		}
		if (error_code)
			break;
	}
	return error_code;
}