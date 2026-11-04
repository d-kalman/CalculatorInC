#pragma once
#define MAX_NUMBER_LENGTH 20
#define MAX_EXPRESSION_LENGTH 120
#include "token.h"
#include "stack.h"

typedef struct AnswerState_t {
	double previous_answer;
	int has_previous_answer;
} AnswerState;

typedef struct Result_t {
	double result;
	enum error_code error_code;
} Result;

typedef struct Parse_result_t {
	Token* tokens;
	int tokens_size;
	enum error_code error_code;
} ParseResult;

typedef struct Parser_state_t {
	Token* tokens;
	size_t capacity;
	int end;
	Stack* operator_stack_p;
	int operator_expected;
} ParserState;

typedef struct Input_t{
	const char* line;
	int index;
	size_t size;
	size_t capacity;
} Input;

int operator_precedence(char op);
int operator_precedes(char op1, char op2, char associativity);
char operator_associativity(char op);

int parse(const char* string, size_t size, Token ** out_tokens, int* out_parsed_size, AnswerState ans_state);
int process_line(const char* string, int size, double* answer, int* has_previous_answer);
int resolve(Token* tokens, double* out_answer);