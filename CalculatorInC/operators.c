#include "operators.h"
#include <math.h>
static char operator_associativity(char op) {
	char associativity = 'L';
	if (op == '^')
		associativity = 'R';
	return associativity;
}

static int operator_precedence(char op) {
	switch (op) {
	case '^':
		return 4;
	case '~':
		return 3;
	case '*':
	case '/':
		return 2;
	case '+':
	case '-':
		return 1;
	default:
		return 0;
	}
}
int operator_precedes(char op1, char op2) {
	int preceeds = 0;
	char associativity = operator_associativity(op2);
	if (associativity == 'R') {
		if (operator_precedence(op1) > operator_precedence(op2))
			preceeds = 1;
		else
			preceeds = 0;
	}
	else {
		if (operator_precedence(op1) >= operator_precedence(op2)) {
			preceeds = 1;
		}
		else {
			preceeds = 0;
		}
	}
	return preceeds;
}


static double resolve_operation(double lo, double ro, char operation)
{
	switch (operation) {
	case '^':
		return pow(lo, ro);
	case '~':
		return -1 * ro;
	case '+':
		return lo + ro;
	case '-':
		return lo - ro;
	case '*':
		return lo * ro;
	case '/':
		return lo / ro;
	default:
		return 0.0;
	}
}

static int validate_logic(double lo, double ro, char operation) {
	//Division by 0
	if (ro == 0 && operation == '/')
		return 0;
	//Non integer root of a negative number is not allowed
	else if (operation == '^' && lo < 0.0) {
		// Check if exponent is integer
		double intpart;
		if (modf(fabs(ro), &intpart) != 0.0)
			return 0;
		else
			return 1;
	}
	else {
		return 1;
	}
}

static int operands_needed(const Token* t) {
	if (t->operation == '~')
		return 1;
	else if (t->type == TOK_FUNCTION)
		return 1;
	else if (t->type == TOK_OPERATOR)
		return 2;
	else
		return 0;
}

static ErrorCode validate_stack_and_pop(Stack* stack, const Token* t, double* lo, double* ro) {
	int error_code = NO_ERROR;
	int needed = operands_needed(t);
	if (needed == 0 || stack->top + 1 < needed) {
		error_code = PARSING_ERROR;
	}
	else {
		if (needed >= 1)
			*ro = pop_stack(stack).number;
		if (needed >= 2)
			*lo = pop_stack(stack).number;
	}
	return error_code;
}

ErrorCode apply_operator(Stack* stack, Token t) {
	double ro = 0;
	double lo = 0;
	ErrorCode error_code = validate_stack_and_pop(stack, &t, &lo, &ro);
	if (error_code)
		return error_code;
	if (!validate_logic(lo, ro, t.operation)) {
		error_code = LOGICAL_ERROR;
		return error_code;
	}
	Token result = create_num_token(resolve_operation(lo, ro, t.operation));
	if (push_to_stack(stack, result)) {
		error_code = MEMORY_ERROR;
		return error_code;
	}
	return error_code;
}