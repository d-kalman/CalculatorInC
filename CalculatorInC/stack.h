#pragma once

#include "token.h"
#include "calculator_util.h"

typedef struct Stack_t {
	Token token_stack[MAX_STACK_SIZE];
	int top;
} Stack;

void initialize_stack(Stack* stack);
int push_to_stack(Stack* stack, Token p);
Token pop_stack(Stack* stack);

int is_full(Stack* stack);
int is_empty(Stack* stack);
Token inspect(Stack* stack);

ErrorCode pop_operands(Stack* stack, int count, double* out);

void stack_tests();
