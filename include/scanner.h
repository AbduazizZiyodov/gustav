#pragma once

#include <stddef.h>

#include "common.h"

#define FOREACH_TOKEN_TYPE(DO)  \
	DO(TOKEN_LEFT_PAREN)    \
	DO(TOKEN_RIGHT_PAREN)   \
	DO(TOKEN_LEFT_BRACE)    \
	DO(TOKEN_RIGHT_BRACE)   \
	DO(TOKEN_COMMA)         \
	DO(TOKEN_DOT)           \
	DO(TOKEN_MINUS)         \
	DO(TOKEN_PLUS)          \
	DO(TOKEN_SEMICOLON)     \
	DO(TOKEN_SLASH)         \
	DO(TOKEN_STAR)          \
	DO(TOKEN_BANG)          \
	DO(TOKEN_BANG_EQUAL)    \
	DO(TOKEN_EQUAL)         \
	DO(TOKEN_EQUAL_EQUAL)   \
	DO(TOKEN_GREATER)       \
	DO(TOKEN_GREATER_EQUAL) \
	DO(TOKEN_LESS)          \
	DO(TOKEN_LESS_EQUAL)    \
	DO(TOKEN_IDENTIFIER)    \
	DO(TOKEN_STRING)        \
	DO(TOKEN_NUMBER)        \
	DO(TOKEN_AND)           \
	DO(TOKEN_CLASS)         \
	DO(TOKEN_ELSE)          \
	DO(TOKEN_FALSE)         \
	DO(TOKEN_FOR)           \
	DO(TOKEN_FUN)           \
	DO(TOKEN_IF)            \
	DO(TOKEN_NIL)           \
	DO(TOKEN_OR)            \
	DO(TOKEN_PRINT_STDOUT)  \
	DO(TOKEN_PRINT_STDERR)  \
	DO(TOKEN_RETURN)        \
	DO(TOKEN_SUPER)         \
	DO(TOKEN_THIS)          \
	DO(TOKEN_TRUE)          \
	DO(TOKEN_VAR)           \
	DO(TOKEN_WHILE)         \
	DO(TOKEN_ERROR)         \
	DO(TOKEN_POW)           \
	DO(TOKEN_PLUS_PLUS)     \
	DO(TOKEN_LOOP)          \
	DO(TOKEN_BREAK)         \
	DO(TOKEN_CONTINUE)      \
	DO(TOKEN_EOF)

typedef enum { FOREACH_TOKEN_TYPE(GENERATE_ENUM) } TokenType;
static const char *TOKEN_TYPE_STRING[] = { FOREACH_TOKEN_TYPE(GENERATE_STRING) };

typedef struct {
	TokenType type;
	const char *start;
	size_t length;
	size_t line;
} Token;

void init_scanner(const char *source);
Token scan_token(void);

typedef struct {
	const char *value;
	size_t length;
	TokenType type;
} Keyword;

static Keyword keywords_table[] = {
	{ "and", 3, TOKEN_AND },
	{ "break", 5, TOKEN_BREAK },
	{ "continue", 8, TOKEN_CONTINUE },
	{ "class", 5, TOKEN_CLASS },
	{ "else", 4, TOKEN_ELSE },
	{ "false", 5, TOKEN_FALSE },
	{ "for", 3, TOKEN_FOR },
	{ "fun", 3, TOKEN_FUN },
	{ "if", 2, TOKEN_IF },
	{ "loop", 4, TOKEN_LOOP },
	{ "nil", 3, TOKEN_NIL },
	{ "or", 2, TOKEN_OR },
	{ "return", 6, TOKEN_RETURN },
	{ "stdout", 6, TOKEN_PRINT_STDOUT },
	{ "stderr", 6, TOKEN_PRINT_STDERR },
	{ "super", 5, TOKEN_SUPER },
	{ "this", 4, TOKEN_THIS },
	{ "true", 4, TOKEN_TRUE },
	{ "var", 3, TOKEN_VAR },
	{ "while", 5, TOKEN_WHILE },
};
