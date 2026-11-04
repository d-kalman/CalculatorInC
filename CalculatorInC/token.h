#pragma once
typedef enum {
    TOK_NUMBER,
    TOK_OPERATOR,
    TOK_FUNCTION,   // sin, cos, abs...
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_END
} TokenType;

typedef enum {
    NOT_FUNCTION,
    SIN,
    COS,
    ABS
} FunctionId;

typedef struct Token_t
{
    TokenType token_type;
	double number;
	char operation;
    FunctionId function_id;
} Token;

Token init_token();

int compare_token_list(Token* list1, int size1, Token* list2, int size2);
int compare_token(Token t1, Token t2);
void print_token(Token t);
void print_token_list(Token* tokens, int size);