#include <math.h>
#include <stdio.h>

#include "function.h"

//Ok, so what do I need?
//I need the function implementations
//I have access to the functions through the function pointer
//Now I have to apply said function

typedef ErrorCode(*FunctionFn)(const double* args, int argc, double* out);

typedef struct Function_t {
    FunctionFn function;
    int min_args;
    int max_args;
    const char* name;
} Function;

static ErrorCode apply_sin(const double* args, int argc, double* out) { 
    *out = sin(args[0]);
    return NO_ERROR;
}
static ErrorCode apply_cos(const double* args, int argc, double* out) {
    *out = cos(args[0]);
    return NO_ERROR;
}
static ErrorCode apply_abs(const double* args, int argc, double* out) {
    *out = fabs(args[0]);
    return NO_ERROR;
}

static const Function FUNCTIONS[FUNCTION_COUNT] = {
    [SIN] = { apply_sin, 1, 1, "sin" },
    [COS] = { apply_cos, 1, 1, "cos" },
    [ABS] = { apply_abs, 1, 1, "abs" },
};

static const Function* get_function(FunctionId id) { 
    if (id <= NOT_FUNCTION || id >= FUNCTION_COUNT)
        return NULL;
    return &FUNCTIONS[id];
}

ErrorCode apply_function(Stack* stack, Token t, double* out_result, int argc)
{
	ErrorCode error_code = NO_ERROR;
    if (t.function_id == NOT_FUNCTION)
        return PARSING_ERROR;
    Function* func = get_function(t.function_id);
    if (argc > func->max_args || argc < func->min_args)
        return PARSING_ERROR;

    double* args = calloc(argc, sizeof(double));
    if (args == NULL)
        return MEMORY_ERROR;
    FunctionFn callable = func->function;

    for (int i = argc - 1; i >= 0; --i) {
        Token t = pop_stack(stack);
        if (t.type == TOK_INVALID)
            error_code = PARSING_ERROR;
            break;
    }
    if (!error_code) {
        callable(args, argc, out_result);
    }
    free(args);
	return error_code;
}

int function_test() {
    printf("Function tests\n");
    Stack stack;
    Stack* stack_p = &stack;

    initialize_stack(stack_p);
    Token t = create_num_token(1.57);
    push_to_stack(stack_p, t);
    Token t_op = create_func_token(SIN);
    double result = 0.0;
    apply_function(stack_p, t_op, &result, 1);
    printf("%f\n", result);

    t = create_num_token(-1.57);
    push_to_stack(stack_p, t);
    t_op = create_func_token(ABS);
    apply_function(stack_p, t_op, &result, 1);
    printf("%f\n", result);
    return 0;
}
