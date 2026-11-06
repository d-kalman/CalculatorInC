#pragma once
#define MAX_NUMBER_LENGTH 20
#define MAX_EXPRESSION_LENGTH 120
#include "calculator_util.h"
#include "token.h"
#include "stack.h"

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

typedef struct Parser_state_t {
	TokenList* parsed_tokens;
	TokenList* rpn_tokens;
	Stack* operator_stack_p;
	int operator_expected;
} ParserState;

typedef struct Input_t{
	const char* line;
	size_t index;
	size_t size;
	size_t capacity;
} Input;

int operator_precedence(char op);
int operator_precedes(char op1, char op2, char associativity);
char operator_associativity(char op);

ErrorCode tokenize(Input* input, ParserState* state, size_t begin);
ErrorCode to_rpn(ParserState* state);
ErrorCode parse(const char* string, size_t size, TokenList** out_tokens, AnswerState ans_state);
//Changes answer only on success
ErrorCode process_line(const char* string, int size, double* answer, int* has_previous_answer);
ErrorCode resolve(const TokenList* token_list, double* out_answer);