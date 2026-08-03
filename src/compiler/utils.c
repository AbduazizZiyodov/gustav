#include "chunk.h"
#include "log.h"
#include "scanner.h"

#include "error.h"
#include "internal.h"
#include "utils.h"

Chunk *current_chunk(void)
{
	return &current->function->chunk;
}

void token_advance(void)
{
	parser_state.previous = parser_state.current;

	while (true) {
		parser_state.current = Scanner_ScanToken();
		LOG_DEBUG("line=%04d %-20s <=> '%.*s'\n", parser_state.current.line,
			  TOKEN_TYPE_STRING[parser_state.current.type], parser_state.current.length,
			  parser_state.current.start);

		if (parser_state.current.type != TOKEN_ERROR) {
			break;
		}

		error_at_current(parser_state.current.start);
	}
}

void token_consume(TokenType type, const char *message)
{
	if (parser_state.current.type == type) {
		token_advance();
		return;
	}

	error_at_current(message);
}

bool token_check(TokenType type)
{
	return parser_state.current.type == type;
}

bool token_match(TokenType type)
{
	if (!token_check(type)) {
		return false;
	}
	token_advance();
	return true;
}
