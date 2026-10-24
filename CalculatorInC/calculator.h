#pragma once
#define MAX_NUMBER_LENGTH 20
#define MAX_EXPRESSION_LENGTH 120
#include "parseable.h"

int allowed_input_operation(char c);
int allowed_followup_operation(char c);

int operator_precedence(char op);
int operator_preceeds(char op1, char op2);

int parse(const char* string, int size, Parseable ** out_parseable, int* out_parsed_size, double previous_answer);
int process_line(const char* string, int size, double* answer);
int resolve(Parseable* parseables, double* out_answer);

int validate_logic(double lo, double ro, char operation);