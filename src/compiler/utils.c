#include "chunk.h"
#include "log.h"
#include "scanner.h"

#include "compiler.h"
#include "compiler/error.h"
#include "compiler/utils.h"

chunk_t *current_chunk(void)
{
	return &current->function->chunk;
}

void advance(void)
{
	parser_state.previous = parser_state.current;

	while (true) {
		parser_state.current = scan_token();
		LOG_DEBUG("line=%04d %-20s <=> '%.*s'\n",
			  parser_state.current.line,
			  TOKEN_TYPE_STRING[parser_state.current.type],
			  parser_state.current.length,
			  parser_state.current.start);

		if (parser_state.current.type != TOKEN_ERROR) {
			break;
		}

		error_at_current(parser_state.current.start);
	}
}

void consume(TokenType type, const char *message)
{
	if (parser_state.current.type == type) {
		advance();
		return;
	}

	error_at_current(message);
}

bool check(TokenType type)
{
	return parser_state.current.type == type;
}

bool match(TokenType type)
{
	if (!check(type)) {
		return false;
	}
	advance();
	return true;
}
