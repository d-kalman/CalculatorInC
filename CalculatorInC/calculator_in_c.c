
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>

#include "stack.h"
#include "calculator.h"
#include "calculator_util.h"
#include "calculator_tests.h"
#include "thread_pool.h"
#include "calculator_in_c.h"

#define MAX_LINE_LENGTH 120
#define MAX_NUM_LINES 1000
#define NUM_CORES 4

void interactive_mode_run() {
	char line[MAX_LINE_LENGTH];
	double answer = 0;
	int has_previous_answer = 0;
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
		int error_code = process_line(line, length, &answer, &has_previous_answer);
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

void batch_mode_run(char* input_file_name, char* output_file_name) {
	FILE* input_file_pointer;
	FILE* output_file_pointer;
	int parallel_processing = 1;
	char line[MAX_LINE_LENGTH];

	if (input_file_name != NULL) {
		fopen_s(&input_file_pointer, input_file_name, "r");
		if (input_file_pointer == NULL) {
			printf("Input file error\n");
			return;
		}
	} 
	else {
		input_file_pointer = stdin;
	}

	if (output_file_name != NULL) {
		fopen_s(&output_file_pointer, output_file_name, "w");
		if (output_file_pointer == NULL) {
			printf("Output file error\n");
			return;
		}
	} 
	else {
		output_file_pointer = stdout;
	}
	JobQueue* queue = NULL;
	thrd_t* threads = NULL;
	Task* results = NULL;
	if (parallel_processing) {
		queue = init_queue();
		threads = malloc(NUM_CORES * sizeof(thrd_t));
		results = calloc(MAX_NUM_LINES, sizeof(Task));
		thread_pool_init(threads, NUM_CORES, queue, results);
	}
	int id = 0;
	while (fgets(line, sizeof(line), input_file_pointer)) {
		//TODO: handle too long expression
		/*if (strchr(line, '\n') == NULL && !feof(stdin)) {
			while (getchar() != '\n' && !feof(stdin)) {}
			printf("Expression too long\n");
			continue;
		}*/

		line[strcspn(line, "\r\n")] = '\0';
		//Empty line: continue
		if (line[0] == '\0')
			continue;

		int length = (int)strlen(line);
		double answer = 0;
		int has_answer = 0;
		if (parallel_processing) {
			Task* task = malloc(sizeof(Task));
			task->id = id;
			strcpy_s(task->expression, 120, line);
			enqueue_task(queue, task);
			id++;
		}
		else {
			int error_code = process_line(line, length, &answer, &has_answer);
			print_results(error_code, output_file_pointer, answer);
		}
		if (id > 20)
			break;
	}
	if (input_file_pointer != stdin) {
		fclose(input_file_pointer);
	}

	if (parallel_processing) {
		thread_pool_shutdown(threads, NUM_CORES, queue);
		for (int i = 0; i < id; ++i)
		{	
			print_results(output_file_pointer, results[i].error_code, results[i].result);
		}

		free(results);
		free(queue);
		free(threads);
	}

	if (output_file_pointer != stdout) {
		fclose(output_file_pointer);
	}

	
}

void print_results(FILE* output_file_pointer, int error_code, double answer)
{
	if (error_code == NO_ERROR) {
		fprintf(output_file_pointer, "%.10g, NO_ERROR\n", answer);
	}
	else if (error_code == MEMORY_ERROR) {
		fprintf(output_file_pointer, "NaN, MEMORY_ERROR\n");
	}
	else if (error_code == PARSING_ERROR) {
		fprintf(output_file_pointer, "NaN, Parsing error\n");
	}
	else if (error_code == INPUT_ERROR) {
		fprintf(output_file_pointer, "NaN, Input error\n");
	}
	else {
		fprintf(output_file_pointer, "NaN, Unknown error\n");
	}
}


void test_mode_run() {
	stack_tests();
	run_calculator_test_suite();
	batch_mode_run(".\\test_files\\batch_test.txt", NULL);
}

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "--test") == 0) {
		test_mode_run();
	}
	else if  (argc > 1 && strcmp(argv[1], "--batch") == 0) {
		char* input_file_name = NULL;
		char* output_file_name = NULL;
		//Input filename provided
		if (argc == 3) {
			input_file_name = argv[2];
		}
		//Input and output filename provided
		else if (argc == 4){
			output_file_name = argv[3];
		}
		batch_mode_run(input_file_name, output_file_name);
	}
	else {
		interactive_mode_run();
	}
	return 0;
}
