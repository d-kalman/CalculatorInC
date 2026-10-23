#pragma once

//stack:
//top
//data

#include "parseable.h"
#define MAX_STACK_SIZE 100

typedef struct Stack_t {
	Parseable parseable_stack[MAX_STACK_SIZE];
	int top;
} Stack;

//Functions
//initalize
// -sets the top to -1
//insert element
//pop element
//inspect top element

void initialize_stack(Stack* stack);
int push_to_stack(Stack* stack, Parseable p);
Parseable pop_stack(Stack* stack);

int is_full(Stack* stack);
int is_empty(Stack* stack);
Parseable inspect(Stack* stack);

void stack_tests();
