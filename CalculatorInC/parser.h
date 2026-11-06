#pragma once

#include "calculator_data.h"
#include "calculator_util.h"
#include "token.h"

typedef struct Input_t {
	const char* line;
	size_t index;
	size_t size;
	size_t capacity;
} Input;

ErrorCode add_previous_answer(Input* input, ParserState* state, AnswerState answer_state);
ErrorCode tokenize(Input* input, ParserState* state);