#include "common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"

static bool is_at_end(void);
static void skip_whitespace(void);
static Token make_token(TokenType type);
static char advance(void);
static bool is_alpha(char c);
static Token make_identifier_token(void);
static bool is_digit(char c);
static Token make_number_token(void);
static bool match(char expected);
static Token make_string_token(void);
static Token make_error_token(const char *message);

typedef struct {
	const char *start;
	const char *current;
	size_t line;
} ScannerState;

static ScannerState scanner_state;

void Scanner_Init(const char *source)
{
	scanner_state.start = source;
	scanner_state.current = source;
	scanner_state.line = 1;
}

Token Scanner_ScanToken(void)
{
	skip_whitespace();

	scanner_state.start = scanner_state.current;

	if (is_at_end()) {
		return make_token(TOKEN_EOF);
	}

	char c = advance();

	if (is_alpha(c)) {
		return make_identifier_token();
	}

	if (is_digit(c)) {
		return make_number_token();
	}

	switch (c) {
	case '(':
		return make_token(TOKEN_LEFT_PAREN);
	case ')':
		return make_token(TOKEN_RIGHT_PAREN);
	case '{':
		return make_token(TOKEN_LEFT_BRACE);
	case '}':
		return make_token(TOKEN_RIGHT_BRACE);
	case ';':
		return make_token(TOKEN_SEMICOLON);
	case ',':
		return make_token(TOKEN_COMMA);
	case '.':
		return make_token(TOKEN_DOT);
	case '-':
		return make_token(TOKEN_MINUS);
	case '+':
		return make_token(match('+') ? TOKEN_PLUS_PLUS : TOKEN_PLUS);
	case '/':
		return make_token(TOKEN_SLASH);
	case '*':
		return make_token(match('*') ? TOKEN_POW : TOKEN_STAR);
	case '!':
		return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	case '=':
		return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '<':
		return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
	case '>':
		return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
	case '"':
		return make_string_token();
	default:
		UNREACHABLE();
	}

	return make_error_token("Unexpected character.");
}

static Token make_token(TokenType type)
{
	size_t length = (size_t)(scanner_state.current - scanner_state.start);

	return (Token){ .type = type,
			.start = scanner_state.start,
			.length = length,
			.line = scanner_state.line };
}

static Token make_error_token(const char *message)
{
	return (Token){ .type = TOKEN_ERROR,
			.start = message,
			.length = (size_t)strlen(message),
			.line = scanner_state.line };
}

static bool is_at_end(void)
{
	return *scanner_state.current == '\0';
}

static char advance(void)
{
	scanner_state.current++;
	return scanner_state.current[-1];
}

static char peek(void)
{
	return *scanner_state.current;
}

static char peek_next(void)
{
	if (is_at_end()) {
		return '\0';
	}
	return scanner_state.current[1];
}

static bool match(char expected)
{
	if (is_at_end()) {
		return false;
	}

	if (*scanner_state.current != expected) {
		return false;
	}

	scanner_state.current++;

	return true;
}

static void skip_whitespace(void)
{
	while (true) {
		char c = peek();

		switch (c) {
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		case '\n':
			scanner_state.line++;
			advance();
			break;
		case '/':
			if (peek_next() == '/') {
				while (peek() != '\n' && !is_at_end()) {
					advance();
				}
			} else {
				return;
			}
			break;
		default:
			return;
		}
	}
}

static Token make_string_token(void)
{
	while (peek() != '"' && !is_at_end()) {
		if (peek() == '\n') {
			scanner_state.line++;
		}
		advance();
	}

	if (is_at_end()) {
		return make_error_token("Unterminated string.");
	}
	advance();
	return make_token(TOKEN_STRING);
}

static bool is_digit(char c)
{
	return (bool)(c >= '0' && c <= '9');
}

static Token make_number_token(void)
{
	while (is_digit(peek())) {
		advance();
	}

	if (peek() == '.' && is_digit(peek_next())) {
		advance();
		while (is_digit(peek())) {
			advance();
		}
	}

	return make_token(TOKEN_NUMBER);
}

static bool is_alpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static TokenType get_identifier_type(void)
{
	size_t identifier_length = (size_t)(scanner_state.current - scanner_state.start);

	for (unsigned int idx = 0; idx < ARRAY_LENGTH(keywords_table); idx++) {
		if (identifier_length == keywords_table[idx].length &&
		    memcmp(scanner_state.start, keywords_table[idx].value,
			   keywords_table[idx].length) == 0) {
			return keywords_table[idx].type;
		}
	}

	return TOKEN_IDENTIFIER;
}

static Token make_identifier_token(void)
{
	while (is_alpha(peek()) || is_digit(peek())) {
		advance();
	}

	return make_token(get_identifier_type());
}
