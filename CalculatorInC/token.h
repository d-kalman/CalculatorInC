#pragma once
typedef struct Token_t
{
	double number;
	char operation;
} Token;
Token init_token();

int compare_token_list(Token* list1, int size1, Token* list2, int size2);
int compare_token(Token t1, Token t2);
void print_token(Token t);
void print_token_list(Token* tokens, int size);