#include "call_stack.h"

int push_call(CallStack* stack, CallFrame frame)
{
	if (!stack || stack->count >= MAX_CALL_DEPTH)
		return 1;
	stack->frames[stack->count++] = frame;
	return 0;
}

CallFrame pop_call(CallStack* stack)
{
	CallFrame frame = { 0 };
	if (!stack || stack->count == 0)
		return frame;
	return stack->frames[--stack->count];
}

CallFrame* inspect_call(CallStack* stack)
{
	if (!stack || stack->count == 0)
		return NULL;
	return &stack->frames[stack->count - 1];
}
