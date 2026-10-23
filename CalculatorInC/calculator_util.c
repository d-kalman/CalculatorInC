#include "calculator_util.h"

int float_compare(double left, double right) {
	return (fabs(left - right) <= 1e-9);
}