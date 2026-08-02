#include "chunk.h"
#include "scanner.h"

#include "compiler/byte_code.h"
#include "compiler/parse.h"
#include "compiler/print.h"
#include "compiler/utils.h"

void print_statement(void)
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emit_byte(OP_PRINT);
}
