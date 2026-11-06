#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "stack.h"
#include "calculator.h"

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
	case '*':
	case '/':
		return 2;
	case '+':
	case '-':
		return 1;
	default:
		return 0;
	}
}

static FunctionId get_function_id(char* func_name) {
	if (strcmp(func_name, "sin") == 0)
		return SIN;
	else if(strcmp(func_name, "cos") == 0)
		return COS;
	else if (strcmp(func_name, "abs") == 0)
		return ABS;
	else
		return NOT_FUNCTION;
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

static ErrorCode drain_operator_stack(ParserState* state) {
	ErrorCode error_code = NO_ERROR;
	while (!is_empty(state->operator_stack_p)) {
		//Unmatched bracket, error
		if (inspect(state->operator_stack_p).operation == '(') {
			error_code = PARSING_ERROR;
			break;
		}
		else {
			error_code = add_token(state->rpn_tokens, pop_stack(state->operator_stack_p));
			if (error_code)
				break;
		}
	}
	return error_code;
}

static ErrorCode parse_arithmetic_operator(ParserState* state, Token t) {
	ErrorCode error_code = NO_ERROR;
	
	if (state->operator_expected == 0)
		return PARSING_ERROR;

	while (!is_empty(state->operator_stack_p) &&
		operator_precedes(inspect(state->operator_stack_p).operation,
			t.operation,
			operator_associativity(t.operation)))
	{
		error_code = add_token(state->rpn_tokens, pop_stack(state->operator_stack_p));
		if (error_code)
			return error_code;
	}
	if (push_to_stack(state->operator_stack_p, t))
		error_code = MEMORY_ERROR;
	state->operator_expected = 0;
	return error_code;
}

static ErrorCode parse_unary(ParserState* state, Token t) {
	ErrorCode error_code = NO_ERROR;
	if (t.operation == '-') {
		if (state->operator_expected) {
			error_code = parse_arithmetic_operator(state, t);
		}
		//Unary operator
		else {
			t.operation = '~';
			if (push_to_stack(state->operator_stack_p, t))
				error_code = MEMORY_ERROR;
		}
	}

	else if (t.operation == '+') {
		//Unary + operator: skip
		//TODO: Decide if we want to do something with this.
		if (state->operator_expected)
			error_code = parse_arithmetic_operator(state, t);
	}
	state->operator_expected = 0;
	return error_code;
}

static ErrorCode parse_close_paren(ParserState* state) {
	ErrorCode error_code = NO_ERROR;
	if (state->operator_expected == 0)
		return PARSING_ERROR;
	while (!is_empty(state->operator_stack_p) && inspect(state->operator_stack_p).operation != '(') {
		error_code = add_token(state->rpn_tokens, pop_stack(state->operator_stack_p));
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

static ErrorCode parse_operator(ParserState* state, Token t) {
	ErrorCode error_code = NO_ERROR;
	if (!allowed_input_operation(t.operation)) {
		error_code = INPUT_ERROR;
	}
	else {
		switch (t.operation) {
		case '-':
		case '+':
			error_code = parse_unary(state, t);
			break;
		case '(':
			if (state->operator_expected == 0) {
				if (push_to_stack(state->operator_stack_p, t)) {
					error_code = MEMORY_ERROR;
				}
				else {
					state->operator_expected = 0;
				}
			}
			else {
				error_code = PARSING_ERROR;
			}
			break;
		case '*':
		case '/':
		case '^':
			error_code = parse_arithmetic_operator(state, t);
			break;
		case ')':
			error_code = parse_close_paren(state);
			break;

		}
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

ErrorCode tokenize(Input* input, ParserState* state, size_t begin) {
	ErrorCode error_code = NO_ERROR;
	for (input->index = begin; input->index < input->size; ++(input->index)) {
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

ErrorCode to_rpn(ParserState* state)
{
	ErrorCode error_code = NO_ERROR;
	TokenList* token_list = state->parsed_tokens;
	state->operator_expected = 0;
	for (int i = 0; i < token_list->end; ++i) {
		Token t = token_list->tokens[i];
		switch (t.type) {
		case TOK_NUMBER:
			if (state->operator_expected == 1)
				error_code = PARSING_ERROR;
			else {
				error_code = add_token(state->rpn_tokens, t);
				state->operator_expected = 1;
			}
			break;
		case TOK_LPAREN:
			if (state->operator_expected == 1)
				error_code = PARSING_ERROR;
			else {
				error_code = parse_operator(state, t);
			}
			break;
		case TOK_OPERATOR:
		case TOK_RPAREN:
			error_code = parse_operator(state, t);
		}
		if (error_code)
			break;
	}
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
	case '*':
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

ErrorCode parse(const char* l_string, size_t l_size, TokenList** out_tokens, AnswerState answer_state) {

	Input input = { .line = l_string, .size = l_size, .capacity = l_size + 2, .index = 0};
	TokenList* parsed_tokens = create_token_list(input.capacity);

	if (parsed_tokens == NULL) {
		*out_tokens = NULL;
		return MEMORY_ERROR;
	}
	TokenList* rpn_tokens = create_token_list(input.capacity);
	if (rpn_tokens == NULL) {
		free_token_list(parsed_tokens);
		*out_tokens = NULL;
		return MEMORY_ERROR;
	}

	Stack operator_stack;
	Stack* operator_stack_p = &operator_stack;
	initialize_stack(operator_stack_p);

	ParserState parser_state = {.parsed_tokens = parsed_tokens,
								.rpn_tokens = rpn_tokens,
								.operator_stack_p = operator_stack_p,
								.operator_expected = 0};
	ParserState* parser_state_p = &parser_state;
	ErrorCode error_code = NO_ERROR;

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
		Token t = create_num_token(answer_state.previous_answer);
		error_code = add_token(parser_state_p->parsed_tokens, t);
		if (error_code != 0)
			goto cleanup_on_error;
	}

	error_code = tokenize(&input, parser_state_p, begin);
	if (error_code)
		goto cleanup_on_error;
	error_code = to_rpn(parser_state_p);
	if (error_code)
		goto cleanup_on_error;

	//State left by trailing operator
	if (parser_state.operator_expected == 0) {
		error_code = PARSING_ERROR;
		goto cleanup_on_error;
	}

	//Empty out the operator stack too
	error_code = drain_operator_stack(parser_state_p);
	if (error_code)
		goto cleanup_on_error;

	*out_tokens = parser_state.rpn_tokens;
	free_token_list(parsed_tokens);
	return NO_ERROR;

cleanup_on_error:
	free_token_list(parsed_tokens);
	free_token_list(rpn_tokens);
	*out_tokens = NULL;
	return error_code;
}

ErrorCode process_line(const char* string, int size, double* answer, int* has_previous_answer)
{
	TokenList* token_list = NULL;
	AnswerState ans_state = { .has_previous_answer = *has_previous_answer, .previous_answer = *answer };
	ErrorCode error_code = parse(string, size, &token_list, ans_state);

	if (error_code == NO_ERROR) {
		error_code = resolve(token_list, answer);
		if (error_code == NO_ERROR) {
			*has_previous_answer = 1;
		}
	}
	if(token_list != NULL)
		free_token_list(token_list);
	return error_code;
}

static int operands_needed(const Token* t) {
	if (t->operation == '~')
		return 1;
	else if (t->type == TOK_FUNCTION)
		return 1;
	else if (t->type == TOK_OPERATOR)
		return 2;
	else
		return 0;
}

static ErrorCode validate_stack_and_pop(Stack* stack, const Token* t, double* lo, double* ro) {
	int error_code = NO_ERROR;
	int needed = operands_needed(t);
	if (needed == 0 || stack->top + 1 < needed) {
		error_code = PARSING_ERROR;
	}
	else {
		if (needed >= 1)
			*ro = pop_stack(stack).number;
		if (needed >= 2)
			*lo = pop_stack(stack).number;
	}
	return error_code;
}

ErrorCode resolve(const TokenList* token_list, double* out_answer)
{
	int index = 0;
	ErrorCode error_code = NO_ERROR;
	Stack stack;
	Stack* stack_p = &stack;
	initialize_stack(stack_p);
	while (index < token_list->end) {
		Token t = token_list->tokens[index];
		if (t.type == TOK_NUMBER) {
			if (push_to_stack(stack_p, t)) {
				error_code = MEMORY_ERROR;
				break;
			}
		}
		else if (t.type == TOK_FUNCTION) {
			//Function support is not implemented yet
			error_code = PARSING_ERROR;
			break;
		}
		else {
			double ro = 0;
			double lo = 0;
			error_code = validate_stack_and_pop(stack_p, &t, &lo, &ro);
			if (error_code)
				break;
			if (!validate_logic(lo, ro, t.operation)) {
				error_code = LOGICAL_ERROR;
				break;
			}
			Token result = create_num_token(resolve_operation(lo, ro, t.operation));
			if (push_to_stack(stack_p, result)) {
				error_code = MEMORY_ERROR;
				break;
			}
		}
		index++;
	}
	if (error_code != NO_ERROR)
		return error_code;
	if (stack_p->top == 0) {
		*out_answer = inspect(stack_p).number;
	}
	else {
		error_code = PARSING_ERROR;
	}
	return error_code;
}
