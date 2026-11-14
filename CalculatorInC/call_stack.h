#pragma once

#include <stddef.h>

#include "calculator_util.h"

#define MAX_CALL_DEPTH MAX_EXPRESSION_LENGTH

typedef struct CallFrame_t {
	FunctionId function_id;
	size_t call_depth;
	int comma_count;
} CallFrame;

typedef struct CallStack_t {
	CallFrame frames[MAX_CALL_DEPTH];
	size_t count;
} CallStack;

//Returns 1 on overflow, 0 on success
int push_call(CallStack* stack, CallFrame frame);
//Returns a zeroed frame when the stack is empty
CallFrame pop_call(CallStack* stack);
//Returns NULL when the stack is empty
CallFrame* inspect_call(CallStack* stack);
