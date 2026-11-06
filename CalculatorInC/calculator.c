#include <assert.h>

#include "calculator.h"
#include "stack.h"
#include "parser.h"
#include "rpn_transformer.h"
#include "operators.h"

ErrorCode parse(const char* l_string, size_t l_size, TokenList** out_tokens, AnswerState answer_state) {

	Input input = { .line = l_string, .size = l_size, .capacity = l_size + 2, .index = 0 };
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

	ParserState parser_state = { .parsed_tokens = parsed_tokens,
								.rpn_tokens = rpn_tokens,
								.operator_stack_p = operator_stack_p,
								.operator_expected = 0 };
	ParserState* parser_state_p = &parser_state;
	ErrorCode error_code = NO_ERROR;

	while (input.index < input.size && input.line[input.index] == ' ') {
		input.index++;
	}
	if (input.index >= input.size) {
		error_code = PARSING_ERROR;
		goto cleanup_on_error;
	}

	error_code = add_previous_answer(&input, parser_state_p, answer_state);
	if (error_code)
		goto cleanup_on_error;
	error_code = tokenize(&input, parser_state_p);
	if (error_code)
		goto cleanup_on_error;
	error_code = to_rpn(parser_state_p);
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

ErrorCode resolve(const TokenList* token_list, double* out_answer)
{
	size_t index = 0;
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
			error_code = apply_operator(stack_p, t);
			if (error_code)
				break;
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
	if (token_list != NULL)
		free_token_list(token_list);
	return error_code;
}