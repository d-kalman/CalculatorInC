#pragma once

#include "calculator_util.h"
#include "stack.h"
#include "token.h"

ErrorCode apply_function(Stack* stack, Token t);
int validate_function_argc(FunctionId id, int argc);