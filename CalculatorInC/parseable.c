#include <stdio.h>

#include "calculator_util.h"
#include "parseable.h"


int compare_token_list(Token* list1, int size1, Token* list2, int size2) {
	if (size1 != size2)
		return 0;
	for (int i = 0; i < size1 - 1; ++i)
		if (!compare_token(list1[i], list2[i])) 
			return 0;
	return list1[size1 - 1].operation == '!' && list2[size2 - 1].operation == '!';
}

int compare_token(Token t1, Token t2) {
	if (float_compare(t1.number, t2.number) && t1.operation == t2.operation)
		return 1;
	else
		return 0;
}

Token init_token()
{
	Token p = { .number = 0, .operation = '?' };
	return p;
}

void print_token(Token t)
{
	if (t.operation == '#')
		printf("%f ", t.number);
	else if (t.operation == '?')
		printf("Undefined parseable ");
	else
		printf("%c ", t.operation);
}

void print_token_list(Token* tokens, int size)
{
	for (int i = 0; i < size; ++i) {
		print_token(tokens[i]);
	}
}