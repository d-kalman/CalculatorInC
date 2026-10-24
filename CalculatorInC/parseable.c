#include <stdio.h>

#include "calculator_util.h"
#include "parseable.h"


int compare_parseable_list(Parseable* list1, int size1, Parseable* list2, int size2) {
	if (size1 != size2)
		return 0;
	for (int i = 0; i < size1 - 1; ++i)
		if (!compare_parseable(list1[i], list2[i])) 
			return 0;
	return list1[size1 - 1].operation == '!' && list2[size2 - 1].operation == '!';
}

int compare_parseable(Parseable p1, Parseable p2) {
	if (float_compare(p1.number, p2.number) && p1.operation == p2.operation)
		return 1;
	else
		return 0;
}

Parseable init_parseable()
{
	Parseable p = { .number = 0, .operation = '?' };
	return p;
}

void print_parseable(Parseable p)
{
	if (p.operation == '#')
		printf("%f ", p.number);
	else if (p.operation == '?')
		printf("Undefined parseable ");
	else
		printf("%c ", p.operation);
}

void print_parseable_list(Parseable* parseables, int size)
{
	for (int i = 0; i < size; ++i) {
		print_parseable(parseables[i]);
	}
}