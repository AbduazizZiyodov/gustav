#include "chunk.h"
#include "scanner.h"

#include "compiler/byte_code.h"
#include "compiler/if.h"
#include "compiler/parse.h"
#include "compiler/utils.h"

void if_statement(void)
{
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ') after condition.");

	int then_jump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement();
	int else_jump = emit_jump(OP_JUMP);

	patch_jump(then_jump);
	emit_byte(OP_POP);

	if (match(TOKEN_ELSE)) {
		statement();
	}

	patch_jump(else_jump);
}
