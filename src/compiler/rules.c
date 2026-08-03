#include <stddef.h>

#include "scanner.h"

#include "class.h"
#include "expression.h"

static const ParseRule rules[] = {
	[TOKEN_LEFT_PAREN] = { expression_grouping, expression_call, PREC_CALL },
	[TOKEN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
	[TOKEN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
	[TOKEN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
	[TOKEN_COMMA] = { NULL, NULL, PREC_NONE },
	[TOKEN_DOT] = { NULL, expression_dot, PREC_CALL },
	[TOKEN_MINUS] = { expression_unary, expression_binary, PREC_TERM },
	[TOKEN_PLUS] = { NULL, expression_binary, PREC_TERM },
	[TOKEN_PLUS_PLUS] = { NULL, expression_binary, PREC_TERM },
	[TOKEN_SEMICOLON] = { NULL, NULL, PREC_NONE },
	[TOKEN_SLASH] = { NULL, expression_binary, PREC_FACTOR },
	[TOKEN_STAR] = { NULL, expression_binary, PREC_FACTOR },
	[TOKEN_BANG] = { expression_unary, NULL, PREC_NONE },
	[TOKEN_BANG_EQUAL] = { NULL, expression_binary, PREC_EQUALITY },
	[TOKEN_EQUAL] = { NULL, NULL, PREC_NONE },
	[TOKEN_EQUAL_EQUAL] = { NULL, expression_binary, PREC_EQUALITY },
	[TOKEN_GREATER] = { NULL, expression_binary, PREC_COMPARISON },
	[TOKEN_GREATER_EQUAL] = { NULL, expression_binary, PREC_COMPARISON },
	[TOKEN_LESS] = { NULL, expression_binary, PREC_COMPARISON },
	[TOKEN_LESS_EQUAL] = { NULL, expression_binary, PREC_COMPARISON },
	[TOKEN_LOOP] = { NULL, expression_binary, PREC_NONE },
	[TOKEN_IDENTIFIER] = { expression_variable, NULL, PREC_NONE },
	[TOKEN_STRING] = { expression_string, NULL, PREC_NONE },
	[TOKEN_NUMBER] = { expression_number, NULL, PREC_NONE },
	[TOKEN_AND] = { NULL, expression_and, PREC_AND },
	[TOKEN_CLASS] = { NULL, NULL, PREC_NONE },
	[TOKEN_ELSE] = { NULL, NULL, PREC_NONE },
	[TOKEN_FALSE] = { expression_literal, NULL, PREC_NONE },
	[TOKEN_FOR] = { NULL, NULL, PREC_NONE },
	[TOKEN_FUN] = { NULL, NULL, PREC_NONE },
	[TOKEN_IF] = { NULL, NULL, PREC_NONE },
	[TOKEN_NIL] = { expression_literal, NULL, PREC_NONE },
	[TOKEN_OR] = { NULL, expression_or, PREC_OR },
	[TOKEN_PRINT_STDOUT] = { NULL, NULL, PREC_NONE },
	[TOKEN_PRINT_STDERR] = { NULL, NULL, PREC_NONE },
	[TOKEN_RETURN] = { NULL, NULL, PREC_NONE },
	[TOKEN_SUPER] = { class_super, NULL, PREC_NONE },
	[TOKEN_THIS] = { class_this, NULL, PREC_NONE },
	[TOKEN_TRUE] = { expression_literal, NULL, PREC_NONE },
	[TOKEN_VAR] = { NULL, NULL, PREC_NONE },
	[TOKEN_WHILE] = { NULL, NULL, PREC_NONE },
	[TOKEN_ERROR] = { NULL, NULL, PREC_NONE },
	[TOKEN_POW] = { NULL, expression_binary, PREC_FACTOR },
	[TOKEN_EOF] = { NULL, NULL, PREC_NONE },
};

const ParseRule *get_rule(TokenType type)
{
	return &rules[type];
}
