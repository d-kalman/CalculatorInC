#include <math.h>

#include "function.h"

typedef ErrorCode (*FunctionFn)(const double* args, int argc, double* out);

typedef struct Function_t {
	FunctionFn fn;
	int min_args;
	int max_args;
	const char* name;
} Function;

static ErrorCode apply_sin(const double* args, int argc, double* out)
{
	(void)argc;
	*out = sin(args[0]);
	return NO_ERROR;
}

static ErrorCode apply_cos(const double* args, int argc, double* out)
{
	(void)argc;
	*out = cos(args[0]);
	return NO_ERROR;
}

static ErrorCode apply_abs(const double* args, int argc, double* out)
{
	(void)argc;
	*out = fabs(args[0]);
	return NO_ERROR;
}

static const Function FUNCTIONS[FUNCTION_COUNT] = {
	[SIN] = { apply_sin, 1, 1, "sin" },
	[COS] = { apply_cos, 1, 1, "cos" },
	[ABS] = { apply_abs, 1, 1, "abs" },
};

static const Function* get_function(FunctionId id)
{
	if (id <= NOT_FUNCTION || id >= FUNCTION_COUNT)
		return NULL;
	return &FUNCTIONS[id];
}
int validate_function_argc(FunctionId id, int argc) {
	if (argc > MAX_FUNCTION_ARGS)
		return 0;
	const Function* fn = get_function(id);
	//Unbounded
	if (fn != NULL && argc >= fn->min_args && (fn->max_args < 0 || argc <= fn->max_args))
		return 1;
	else
		return 0;
}

ErrorCode apply_function(Stack* stack, Token t)
{
	const Function* func = get_function(t.function_id);
	if (func == NULL || func->fn == NULL)
		return PARSING_ERROR;

	//The token carries the arity of this particular call
	int argc = t.argc;
	if(!validate_function_argc(t.function_id, argc)) {
		return ARITY_ERROR;
	}
	double args[MAX_FUNCTION_ARGS] = { 0.0 };
	ErrorCode error_code = pop_operands(stack, argc, args);
	if (error_code)
		return error_code;

	double result = 0.0;
	error_code = func->fn(args, argc, &result);
	if (error_code)
		return error_code;

	if (push_to_stack(stack, create_num_token(result)))
		return MEMORY_ERROR;
	return NO_ERROR;
}
