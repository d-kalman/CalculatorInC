#pragma once

#include "calculator_util.h"
#include "calculator_data.h"
#include "token.h"

ErrorCode parse(const char* string, size_t size, TokenList** out_tokens, AnswerState ans_state);
//Changes answer only on success
ErrorCode process_line(const char* string, size_t size, double* answer, int* has_previous_answer);
ErrorCode resolve(const TokenList* token_list, double* out_answer);