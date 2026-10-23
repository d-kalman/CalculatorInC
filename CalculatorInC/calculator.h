#pragma once
#define MAX_NUMBER_LENGTH 20
#define MAX_EXPRESSION_LENGTH 120
#include "parseable.h"

int allowed_input_operation(char c);
int allowed_followup_operation(char c);

int operator_precedence(char op);
int operator_preceeds(char op1, char op2);

int parse(char* string, int size, Parseable ** out_parseable, int* out_parsed_size, double previous_answer);
double process_line(char* string, int size, double answer);
double resolve(Parseable* parseables);



void operator_tests();
int parsing_tests();
int resolve_tests();