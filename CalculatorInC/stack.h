#pragma once

#include "token.h"
#include "calculator_util.h"

#define MAX_STACK_SIZE 120

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

ErrorCode validate_stack_and_pop(Stack* stack, Token token, double* args);

void stack_tests();
