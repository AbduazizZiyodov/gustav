#pragma once

#include <stddef.h>

// Source - https://stackoverflow.com/a/10966395
// Posted by Terrence M, modified by community. See post 'Timeline' for change
// history Retrieved 2026-01-08, License - CC BY-SA 3.0
#define str(x) #x
#define xstr(x) str(x)
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
	DO(TOKEN_EOF)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum { FOREACH_TOKEN_TYPE(GENERATE_ENUM) } TokenType;

static const char *TOKEN_TYPE_STRING[] = { FOREACH_TOKEN_TYPE(
	GENERATE_STRING) };

typedef struct {
	TokenType type;
	const char *start;
	size_t length;
	size_t line;
} Token;

void Scanner_Init(const char *source);
Token Scanner_ScanToken(void);
