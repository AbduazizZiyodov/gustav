#include <stdint.h>
#include <stdio.h>

#include "internal.h"
#include "log.h"
#include "trace.h"
#include "value.h"

#if DEBUG
#include "debug.h"
#endif

#ifdef DEBUG // DEBUG

void trace(CallFrame *frame)

#else

void trace(CallFrame *frame [[maybe_unused]])

#endif // DEBUG - mark arg as unused on release/non-debug builds
{
// Prints the instruction that currently being executed
// (if enabled) & content of the stack
#ifdef DEBUG
	int offset = (int)(frame->ip - frame->closure->function->chunk.code);
	Debug_DisassembleInstruction(&frame->closure->function->chunk, offset);

	LOG_DEBUG("== [stack] ==\n");
	uint16_t i = 0;

	for (Value *slot = vm.stack; slot < vm.stack_top; i++, slot++) {
		printf("[%d] ", i);
		Value_Print(stdout, *slot);
		(void)putchar('\n');
	}

	LOG_DEBUG("== [/stack] ==\n");
	printf("\n");
#endif // DEBUG
}
