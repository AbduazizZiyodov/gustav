#include <stdint.h>
#include <stdio.h>

#include "internal.h"
#include "log.h"
#include "trace.h"
#include "value.h"

#ifdef GUSTAV_DEBUG
#include "debug.h"

void vm_trace(CallFrame *frame)
{
	// NOTE(Abduaziz): if enabled, prints the instruction that
	// currently being executed & content of the stack

	int offset = (int)(frame->ip - frame->closure->function->chunk.code);
	debug_disassemble_instruction(&frame->closure->function->chunk, offset);

	LOG_DEBUG("== [stack] ==\n");

	uint16_t i = 0;

	for (Value *slot = vm.stack; slot < vm.stack_top; i++, slot++) {
		printf("[%d] ", i);
		print_value(stdout, *slot);
		(void)putchar('\n');
	}

	LOG_DEBUG("== [/stack] ==\n\n");
}

#else
void vm_trace(CallFrame *frame [[maybe_unused]])
{
}

#endif // GUSTAV_DEBUG
