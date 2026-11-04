#include <stdio.h>
#include <assert.h>
#include "stack.h"

void initialize_stack(Stack* stack) {
	stack->top = -1;
	return;
}
int push_to_stack(Stack* stack, Token t) {
	int failed = 0;
	if (is_full(stack)) {
		failed = 1;
	}
	else {
		stack->token_stack[++stack->top] = t;
	}
	return failed;
}

int is_full(Stack* stack) {
	if (stack->top >= MAX_STACK_SIZE - 1)
		return 1;
	else
		return 0;
}
int is_empty(Stack* stack) {
	if (stack->top == -1)
		return 1;
	else
		return 0;
}

Token inspect(Stack* stack)
{
	if (is_empty(stack)) {
		return init_token();
	}
	return stack->token_stack[stack->top];
}

Token pop_stack(Stack* stack) {
	Token p = { .number = 0, .operation = '?' };
	if (is_empty(stack)) {
		return p;
	}
	else {
		p = stack->token_stack[stack->top--];
		return p;
	}
}

void stack_tests()
{
	Stack stack;
	Stack* stack_p = &stack;

	initialize_stack(stack_p);
	Token t = { .number = 2.0, .operation = '#' };
	Token t2 = { .number = 1.0, .operation = '#' };
	Token t3 = { .number = 0.0, .operation = '+' };

	push_to_stack(stack_p, t);
	assert(compare_token(inspect(stack_p), t));
	push_to_stack(stack_p, t2);
	assert(compare_token(inspect(stack_p), t2));
	push_to_stack(stack_p, t3);
	assert(compare_token(inspect(stack_p), t3));

	assert(compare_token(pop_stack(stack_p), t3));
	assert(compare_token(pop_stack(stack_p), t2));
	assert(compare_token(pop_stack(stack_p), t));

	assert(is_empty(stack_p) == 1);
}