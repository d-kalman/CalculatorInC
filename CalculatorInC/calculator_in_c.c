
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stack.h"
#include "calculator.h"

#define TEST false

int run_test() {
	int test_result = 0;
	stack_tests(); //assert test
	operator_tests(); //assert test
	if (parsing_tests()) {
		printf("Parsing tests failed");
		test_result++;
	}
	if (resolve_tests()) {
		printf("Parsing tests failed");
		test_result++;
	}

	return test_result;
}

int main()
{
	if (TEST) {
		if (run_test() == 0)
		{
			printf("Tests are successful");
			return 0;
		}
		else {
			printf("Tests failed");
			return 1;
		}
	}
	else {
		char line[120];
		double answer = 0;
		while (fgets(line, sizeof(line), stdin) != NULL) {
			if (strcmp(line, "exit\n") == 0)
				return 0;
			else {
				int length = (int)strlen(line);
				answer = process_line(line, length-1, answer);
				printf("Answer %f\n", answer);
			}
		}
	}
	return 0;
}

