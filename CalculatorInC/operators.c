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

ErrorCode apply_operator(Stack* stack, Token t) {
	double operands[2] = { 0.0, 0.0 };
	ErrorCode error_code = pop_operands(stack, t.argc, operands);
	if (error_code)
		return error_code;

	double lo = operands[0];
	double ro = (t.argc > 1) ? operands[1] : 0.0;
	if (!validate_logic(lo, ro, t.operation))
		return LOGICAL_ERROR;

	if (push_to_stack(stack, create_num_token(resolve_operation(lo, ro, t.operation))))
		return MEMORY_ERROR;
	return NO_ERROR;
}