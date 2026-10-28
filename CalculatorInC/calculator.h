#pragma once
#define MAX_NUMBER_LENGTH 20
#define MAX_EXPRESSION_LENGTH 120
#include "parseable.h"

typedef struct AnswerState_t {
	double previous_answer;
	int has_previous_answer;
} AnswerState;

typedef struct Result_t {
	double result;
	enum error_code error_code;
} Result;

typedef struct Parse_result_t {
	Parseable* parseables;
	int parseables_size;
	enum error_code error_code;
} ParseResult;

int allowed_input_operation(char c);
int allowed_followup_operation(char c);

int operator_precedence(char op);
int operator_precedes(char op1, char op2, char associativity);
char operator_associativity(char op);

int parse(const char* string, size_t size, Parseable ** out_parseable, int* out_parsed_size, double previous_answer, int has_previous_answer);
int process_line(const char* string, int size, double* answer, int* has_previous_answer);
int resolve(Parseable* parseables, double* out_answer);

int validate_logic(double lo, double ro, char operation);