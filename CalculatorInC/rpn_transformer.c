#include "rpn_transformer.h"
#include "operators.h"
#include "stack.h"
#include "token.h"
#include "function.h"

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

//Pops operators into the RPN list, but keeps the matching '(' on the stack
static ErrorCode pop_operators_to_lparen(ParserState* state) {
	ErrorCode error_code = NO_ERROR;
	while (!is_empty(state->operator_stack_p) &&
		inspect(state->operator_stack_p).operation != '(')
	{
		error_code = add_token(state->rpn_tokens, pop_stack(state->operator_stack_p));
		if (error_code)
			return error_code;
	}
	if (is_empty(state->operator_stack_p))
		return PARSING_ERROR;
	return NO_ERROR;
}

static ErrorCode parse_function(ParserState* state, Token t, int followed_by_lparen) {
	if (!followed_by_lparen || state->operator_expected != 0)
		return PARSING_ERROR;

	//The call's '(' has not been pushed yet, so it will sit at the next depth
	CallFrame frame = {
		.function_id = t.function_id,
		.call_depth = (size_t)state->paren_depth + 1,
		.comma_count = 0
	};
	if (push_call(&state->call_stack, frame))
		return MEMORY_ERROR;
	return NO_ERROR;
}

static ErrorCode parse_close_paren(ParserState* state) {
	ErrorCode error_code = NO_ERROR;
	if (state->operator_expected == 0)
		return PARSING_ERROR;

	//A ')' closes a call when the innermost call started at this depth
	CallFrame* frame = inspect_call(&state->call_stack);
	int is_call = frame != NULL && frame->call_depth == (size_t)state->paren_depth;

	error_code = pop_operators_to_lparen(state);
	if (error_code)
		return error_code;
	pop_stack(state->operator_stack_p);
	state->paren_depth--;

	if (is_call) {
		CallFrame closed = pop_call(&state->call_stack);
		int argc = closed.comma_count + 1;
		if (!validate_function_argc(closed.function_id, argc))
			return ARITY_ERROR;
		Token function_token = create_func_token(closed.function_id, argc);
		error_code = add_token(state->rpn_tokens, function_token);
		if (error_code)
			return error_code;
	}
	state->operator_expected = 1;
	return NO_ERROR;
}

static ErrorCode parse_comma(ParserState* state) {
	if (state->operator_expected == 0)
		return PARSING_ERROR;

	//A comma is only legal directly inside a call, not inside grouping parens
	CallFrame* frame = inspect_call(&state->call_stack);
	if (frame == NULL || frame->call_depth != (size_t)state->paren_depth)
		return PARSING_ERROR;

	ErrorCode error_code = pop_operators_to_lparen(state);
	if (error_code)
		return error_code;
	frame->comma_count++;
	state->operator_expected = 0;
	return NO_ERROR;
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
	state->paren_depth = 0;
	state->call_stack.count = 0;
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
		case TOK_FUNCTION: {
			int followed_by_lparen = (i + 1 < token_list->end) &&
				token_list->tokens[i + 1].type == TOK_LPAREN;
			error_code = parse_function(state, t, followed_by_lparen);
			break;
		}
		case TOK_LPAREN:
			if (state->operator_expected == 1)
				error_code = PARSING_ERROR;
			else {
				error_code = parse_operator(state, t);
				if (!error_code)
					state->paren_depth++;
			}
			break;
		case TOK_COMMA:
			error_code = parse_comma(state);
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
		//State left by a trailing operator
		if (state->operator_expected == 0)
			error_code = PARSING_ERROR;
		else
			error_code = drain_operator_stack(state);
	}
	if (!error_code && state->call_stack.count != 0)
		error_code = PARSING_ERROR;
	return error_code;
}
