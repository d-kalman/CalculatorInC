#pragma once
#include <math.h>

int float_compare(double left, double right);

enum error_code {
	NO_ERROR = 0,
	MEMORY_ERROR = 100,
	PARSING_ERROR = 200,
	INPUT_ERROR = 300,
	LOGICAL_ERROR = 400
};