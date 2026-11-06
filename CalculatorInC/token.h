#pragma once

#define MAX_FUNCTION_ARGS 8

#include <stddef.h>
#include "calculator_util.h"

typedef enum {
    TOK_NUMBER,
    TOK_OPERATOR,
    TOK_FUNCTION,   // sin, cos, abs...
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_INVALID
} TokenType;

typedef struct Token_t
{
    TokenType type;
	double number;
	char operation;
    FunctionId function_id;
} Token;


typedef struct TokenList_t 
{
    Token* tokens;
    size_t capacity;
    size_t end;
} TokenList;

TokenList* create_token_list(size_t capacity);
void free_token_list(TokenList* tl);
Token create_token();
Token create_num_token(double num);
Token create_op_token(char c);
Token create_func_token(FunctionId function_id);

ErrorCode add_token(TokenList* tl, Token t);
int compare_token(Token t1, Token t2);
int compare_token_list(TokenList* list1, TokenList* list2);
void print_token(Token t);
void print_token_list(TokenList* tokens);