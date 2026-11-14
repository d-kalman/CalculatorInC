#pragma once
#include "token.h"
#include "stack.h"
#include "call_stack.h"

typedef struct Parser_state_t {
	TokenList* parsed_tokens;
	TokenList* rpn_tokens;
	Stack* operator_stack_p;
	int operator_expected;
	int paren_depth;
	CallStack call_stack;

} ParserState;

typedef struct AnswerState_t {
	double previous_answer;
	int has_previous_answer;
} AnswerState;

typedef struct Result_t {
	double result;
	ErrorCode error_code;
} Result;

typedef struct Parse_result_t {
	Token* tokens;
	size_t tokens_size;
	ErrorCode error_code;
} ParseResult;
