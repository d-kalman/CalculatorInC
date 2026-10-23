#include <stdio.h>
#include <assert.h>
#include "stack.h"

void initialize_stack(Stack* stack) {
	stack->top = -1;
	return;
}
int push_to_stack(Stack* stack, Parseable p) {
	int error_code = 0;
	int top = stack->top;
	if (is_full(stack)) {
		error_code = 1;
	}
	else {
		top++;
		stack->parseable_stack[top] = p;
		stack->top++;
	}
	return error_code;
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

Parseable inspect(Stack* stack)
{
	if (is_empty(stack)) {
		return init_parseable();
	}
	return stack->parseable_stack[stack->top];
}

Parseable pop_stack(Stack* stack) {
	Parseable p = { .number = 0, .operation = '?' };
	if (is_empty(stack)) {
		return p;
	}
	else {
		int top = stack->top;
		p = stack->parseable_stack[top];
		stack->top--;
		return p;
	}
}

void stack_tests()
{
	Stack stack;
	Stack* stack_p = &stack;

	initialize_stack(stack_p);
	Parseable p = { .number = 2.0, .operation = '#' };
	Parseable p2 = { .number = 1.0, .operation = '#' };
	Parseable p3 = { .number = 0.0, .operation = '+' };

	push_to_stack(stack_p, p);
	assert(compare_parseable(inspect(stack_p), p));
	push_to_stack(stack_p, p2);
	assert(compare_parseable(inspect(stack_p), p2));
	push_to_stack(stack_p, p3);
	assert(compare_parseable(inspect(stack_p), p3));

	assert(compare_parseable(pop_stack(stack_p), p3));
	assert(compare_parseable(pop_stack(stack_p), p2));
	assert(compare_parseable(pop_stack(stack_p), p));

	assert(is_empty(stack_p) == 1);
}