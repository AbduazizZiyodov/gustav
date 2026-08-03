#include <stdio.h>

#include "error.h"
#include "internal.h"
#include "scanner.h"

void error_at(Token *token, const char *message)
{
	if (parser_state.panic_mode) {
		return;
	}

	parser_state.panic_mode = true;

	(void)fprintf(stderr, "[at line %lu] Error", token->line);

	if (token->type == TOKEN_EOF) {
		(void)fprintf(stderr, " at end");
	} else if (token->type == TOKEN_ERROR) {
		//
	} else {
		(void)fprintf(stderr, " at '%.*s'", (int)token->length, token->start);
	}

	(void)fprintf(stderr, ": %s\n", message);

	parser_state.had_error = true;
}

void compiler_error(const char *message)
{
	error_at(&parser_state.previous, message);
}

void error_at_current(const char *what)
{
	error_at(&parser_state.current, what);
}
