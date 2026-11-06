#include "rpn_transformer.h"
#include "operators.h"
#include "stack.h"
#include "token.h"

static ErrorCode parse_arithmetic_operator(ParserState* state, Token t) {
	ErrorCode error_code = NO_ERROR;

	if (state->operator_expected == 0)
		return PARSING_ERROR;

	while (!is_empty(state->operator_stack_p) &&
		operator_precedes(inspect(state->operator_stack_p).operation,
			t.operation))
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
			t.argc = 1;
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
	default:
		error_code = PARSING_ERROR;
		break;
	}
	return error_code;
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

ErrorCode to_rpn(ParserState* state)
{
	ErrorCode error_code = NO_ERROR;
	TokenList* token_list = state->parsed_tokens;
	state->operator_expected = 0;
	for (size_t i = 0; i < token_list->end; ++i) {
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
			break;
		default: 
			error_code = PARSING_ERROR;
		}
		if (error_code)
			break;
	}

	if (!error_code) {
		//State left by trailing operator
		if (state->operator_expected == 0) {
			error_code = PARSING_ERROR;
		}
		else if (!error_code) {
			//Empty out the operator stack too
			error_code = drain_operator_stack(state);
		}
	}
	return error_code;
}