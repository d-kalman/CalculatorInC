#pragma once

#include <stdlib.h>

#include "calculator_util.h"
#include "stack.h"
#include "token.h"

ErrorCode apply_function(Stack* stack, Token t, double* out_result, int argc);
int function_test();