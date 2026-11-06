#pragma once

#include "token.h"
#include "stack.h"

//returns 1 if op1 should be popped before pushing op2 (uses op2's associativity)
int operator_precedes(char op1, char op2);
ErrorCode apply_operator(Stack* stack, Token t);