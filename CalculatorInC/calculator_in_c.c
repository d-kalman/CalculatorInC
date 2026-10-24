
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stack.h"
#include "calculator.h"
#include "calculator_util.h"
#include "calculator_tests.h"

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "--test") == 0) {
		stack_tests();
		run_calculator_test_suite();
	}
	else {
		char line[120];
		double answer = 0;
		while (fgets(line, sizeof(line), stdin) != NULL) {
			if (strchr(line, '\n') == NULL && !feof(stdin)) {
				while (getchar() != '\n' && !feof(stdin)) {}
				printf("Expression too long\n");
				continue;
			}
			line[strcspn(line, "\r\n")] = '\0';

			//Exit the loop
			if (strcmp(line, "exit") == 0)
				break;
			//Empty line: continue
			if (line[0] == '\0')
				continue;

			int length = (int)strlen(line);
			int error_code = process_line(line, length, &answer);
			if (error_code == NO_ERROR) {
				printf("Answer: %.10g\n", answer);
			}
			else if (error_code == MEMORY_ERROR) {
				printf("Memory allocation error\n");
			}
			else if (error_code == PARSING_ERROR) {
				printf("Parsing error\n");
			}
			else if (error_code == INPUT_ERROR) {
				printf("Input error\n");
			}
			else {
				printf("Unknown error\n");
			}
		}
	}
	return 0;
}

