#include "operators.h"
#include <math.h>
#include <stdlib.h>

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
		return -1 * lo;
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

int operands_needed(const Token* t) {
	if (t->operation == '~')
		return 1;
	else if (t->type == TOK_OPERATOR)
		return 2;
	else
		return 0;
}


ErrorCode apply_operator(Stack* stack, Token t) {
	double ro = 0;
	double lo = 0;
	double operands[] = { 0.0, 0.0 };
	if (operands == NULL)
		return MEMORY_ERROR;
	ErrorCode error_code = validate_stack_and_pop(stack, t, operands);
	if (error_code)
		goto cleanup_on_error;
	if (t.argc > 0)
		lo = operands[0];
	if (t.argc > 1)
		ro = operands[1];

	if (!validate_logic(lo, ro, t.operation)) {
		error_code = LOGICAL_ERROR;
		goto cleanup_on_error;
	}
	Token result = create_num_token(resolve_operation(lo, ro, t.operation));
	if (push_to_stack(stack, result)) {
		error_code = MEMORY_ERROR;
		goto cleanup_on_error;
	}
cleanup_on_error:
	return error_code;
}