#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "scanner.h"

typedef struct {
	const char *start;
	const char *current;
	size_t line;
} ScannerState;

static ScannerState scanner_state;

void init_scanner(const char *source)
{
	scanner_state.start = source;
	scanner_state.current = source;
	scanner_state.line = 1;
}

static Token make_token(TokenType type)
{
	return (Token){ .type = type,
			.start = scanner_state.start,
			.length = (size_t)(scanner_state.current -
					   scanner_state.start),
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
	if (is_at_end())
		return '\0';
	return scanner_state.current[1];
}

static bool match(char expected)
{
	if (is_at_end())
		return false;
	if (*scanner_state.current != expected)
		return false;
	scanner_state.current++;
	return true;
}

static void skip_whitespace(void)
{
	for (;;) {
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
				while (peek() != '\n' && !is_at_end())
					advance();
			} else {
				return;
			}
		default:
			return;
		}
	}
}

static Token make_string_token(void)
{
	while (peek() != '"' && !is_at_end()) {
		if (peek() == '\n')
			scanner_state.line++;
		advance();
	}

	if (is_at_end())
		return make_error_token("Unterminated string.");
	advance();
	return make_token(TOKEN_STRING);
}

static bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

static Token make_number_token(void)
{
	while (is_digit(peek()))
		advance();

	if (peek() == '.' && is_digit(peek_next())) {
		advance();
		while (is_digit(peek()))
			advance();
	}

	return make_token(TOKEN_NUMBER);
}

static bool is_alpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static TokenType check_keyword(size_t start, size_t length, const char *rest,
			       TokenType match_type)
{
	bool length_matches = (size_t)(scanner_state.current -
				       scanner_state.start) == start + length;
	bool rest_matches = memcmp(scanner_state.start + start, rest, length) ==
			    0;

	if (length_matches && rest_matches)
		return match_type;
	return TOKEN_IDENTIFIER;
}

static TokenType get_identifier_type(void)
{
	switch (scanner_state.start[0]) {
	case 'a':
		return check_keyword(1, 2, "nd", TOKEN_AND);
	case 'c':
		return check_keyword(1, 4, "lass", TOKEN_CLASS);
	case 'e':
		return check_keyword(1, 3, "lse", TOKEN_ELSE);
	case 'f':
		if (scanner_state.current - scanner_state.start > 1) {
			switch (scanner_state.start[1]) {
			case 'a':
				return check_keyword(2, 3, "lse", TOKEN_FALSE);
			case 'o':
				return check_keyword(2, 1, "r", TOKEN_FOR);
			case 'u':
				return check_keyword(2, 1, "n", TOKEN_FUN);
			}
		}
		break;
	case 'i':
		return check_keyword(1, 1, "f", TOKEN_IF);
	case 'n':
		return check_keyword(1, 2, "il", TOKEN_NIL);
	case 'o':
		return check_keyword(1, 1, "r", TOKEN_OR);
	case 'p':
		return check_keyword(1, 4, "rint", TOKEN_PRINT);
	case 'r':
		return check_keyword(1, 5, "eturn", TOKEN_RETURN);
	case 's':
		return check_keyword(1, 4, "uper", TOKEN_SUPER);
	case 't':
		if (scanner_state.current - scanner_state.start > 1) {
			switch (scanner_state.start[1]) {
			case 'h':
				return check_keyword(2, 2, "is", TOKEN_THIS);
			case 'r':
				return check_keyword(2, 2, "ue", TOKEN_TRUE);
			}
		}
		break;
	case 'v':
		return check_keyword(1, 2, "ar", TOKEN_VAR);
	case 'w':
		return check_keyword(1, 4, "hile", TOKEN_WHILE);
	}

	return TOKEN_IDENTIFIER;
}

static Token make_identifier_token(void)
{
	while (is_alpha(peek()) || is_digit(peek()))
		advance();

	return make_token(get_identifier_type());
}

Token scan_token(void)
{
	skip_whitespace();

	scanner_state.start = scanner_state.current;

	if (is_at_end())
		return make_token(TOKEN_EOF);

	char c = advance();

	if (is_alpha(c))
		return make_identifier_token();
	if (is_digit(c))
		return make_number_token();

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
		return make_token(TOKEN_PLUS);
	case '/':
		return make_token(TOKEN_SLASH);
	case '*':
		return make_token(TOKEN_STAR);
	case '!':
		return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	case '=':
		return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '<':
		return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
	case '>':
		return make_token(match('=') ? TOKEN_GREATER_EQUAL :
					       TOKEN_GREATER);
	case '"':
		return make_string_token();
	}

	return make_error_token("Unexpected character.");
}
