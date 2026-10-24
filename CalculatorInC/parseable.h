#pragma once
typedef struct Parseable_t
{
	double number;
	char operation;
} Parseable;
Parseable init_parseable();

int compare_parseable_list(Parseable* list1, int size1, Parseable* list2, int size2);
int compare_parseable(Parseable p1, Parseable p2);
void print_parseable(Parseable p);
void print_parseable_list(Parseable* parseables, int size);