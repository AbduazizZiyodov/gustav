#include <stdio.h>

#include "compiler.h"
#include "log.h"
#include "scanner.h"

void compile(const char *source)
{
	init_scanner(source);
	LOG_DEBUG("Scanner was initialized");

	for (;;) {
		Token token = scan_token();

		printf("[%lu] %s '%.*s'\n", token.line,
		       TOKEN_TYPE_STRING[token.type], (int)token.length,
		       token.start);

		if (token.type == TOKEN_EOF)
			break;
	}
}
