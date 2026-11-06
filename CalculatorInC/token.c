#include <stdio.h>
#include <stdlib.h>

#include "calculator_util.h"
#include "token.h"


int compare_token_list(TokenList* list1, TokenList* list2) {
	if (!list1 || !list2)
		return 0;
	if (list1->end != list2->end)
		return 0;
	for (int i = 0; i < list1->end; ++i)
		if (!compare_token(list1->tokens[i], list2->tokens[i]))
			return 0;
	//return list1[size1 - 1].operation == '!' && list2[size2 - 1].operation == '!';
	return 1;
}

int compare_token(Token t1, Token t2) {
	if (
		t1.type == t2.type &&
		float_compare(t1.number, t2.number) &&
		t1.operation == t2.operation &&
		t1.function_id == t2.function_id
		)
		return 1;
	else
		return 0;
}

TokenList* create_token_list(size_t capacity)
{
	TokenList* tl = malloc(sizeof(TokenList));
	if (tl == NULL)
		return NULL;
	else {
		tl->tokens = malloc(capacity * sizeof(Token));
		if (tl->tokens == NULL) {
			free(tl);
			return NULL;
		}
	}
	tl->end = 0;
	tl->capacity = capacity;
	return tl;
}
void free_token_list(TokenList* tl) {
	if (tl && tl->tokens) {
		free(tl->tokens);
		free(tl);
	}
}

Token create_token()
{
	Token t = {.type = TOK_INVALID, .number = 0, .operation = '?', .function_id = NOT_FUNCTION };
	return t;
}

Token create_num_token(double num)
{
	Token t = { .type = TOK_NUMBER, .number = num, .operation = '#', .function_id = NOT_FUNCTION };
	return t;
}

Token create_func_token(FunctionId function_id)
{
	Token t = {.type = TOK_FUNCTION, .number = 0, .operation = 'f', .function_id = function_id };
	return t;
}

Token create_op_token(char c)
{
	Token t = { .number = 0, .operation = c, .function_id = NOT_FUNCTION };
	switch (c) {
	case '(':
		t.type = TOK_LPAREN;
		break;
	case ')':
		t.type = TOK_RPAREN;
		break;
	default:
		t.type = TOK_OPERATOR;
	}
	return t;
}

ErrorCode add_token(TokenList* token_list, Token t) {
	ErrorCode error_code = NO_ERROR;
	if (token_list->end >= token_list->capacity - 1) {
		error_code = MEMORY_ERROR;
	}
	else {
		token_list->tokens[token_list->end++] = t;
	}
	return error_code;
}

void print_token(Token t)
{
	if (t.type == TOK_NUMBER)
		printf("%f ", t.number);
	else if (t.type == TOK_INVALID)
		printf("Undefined parseable ");
	else if (t.type == TOK_OPERATOR)
		printf("%c ", t.operation);
	else if (t.type == TOK_FUNCTION)
		printf("%s ", "Function");
	else
		printf("Unknown\n");
}

void print_token_list(TokenList* token_list)
{
	for (int i = 0; i < token_list->end; ++i) {
		print_token(token_list->tokens[i]);
	}
}