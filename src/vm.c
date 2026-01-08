#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "log.h"
#include <stdbool.h>
#include <stdio.h>

#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif

static VM vm;

void push(Value value)
{
	*vm.stack_top = value;
	vm.stack_top++;
}

Value pop(void)
{
	vm.stack_top--;
	return *vm.stack_top;
}

static void reset_stack(void)
{
	vm.stack_top = vm.stack;
}

void init_vm(void)
{
	LOG_INFO("VM initialized");
	reset_stack();
}

void free_vm(void)
{
	LOG_INFO("VM freed");
}

static void trace(bool disassemble)
{
	// Prints the instruction that currently being executed (if enabled) &
	// content of the stack

	if (disassemble) {
		size_t offset = (size_t)(vm.ip - vm.chunk->code);
		disassemble_instruction(vm.chunk, offset);
	}

	LOG_TRACE("== Stack ==");
	uint16_t i = 0;
	for (Value *slot = vm.stack; slot < vm.stack_top; i++, slot++) {
		LOG_TRACE("[%d] %g", i, *slot);
	}
	LOG_TRACE("== Stack END ==");
	printf("\n");
}

static InterpretResult run(void)
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)             \
	do {                      \
		double b = pop(); \
		double a = pop(); \
		push(a op b);     \
	} while (false);

	Value constant, top_value;
	uint8_t instruction;

	printf("\n");

	while (true) {
#ifdef DEBUG_TRACE_EXECUTION
		trace(true);
#endif

		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT:
			constant = READ_CONSTANT();
			push(constant);
			break;
		case OP_ADD:
			BINARY_OP(+);
			break;
		case OP_SUBTRACT:
			BINARY_OP(-);
			break;
		case OP_MULTIPLY:
			BINARY_OP(*);
			break;
		case OP_DIVIDE:
			BINARY_OP(/);
			break;
		case OP_NEGATE:
			// doing it in-place - no pop/push
			top_value = *(vm.stack_top - 1);
			*(vm.stack_top - 1) = -top_value;
			break;
		case OP_RETURN:
			LOG_TRACE("RETURN => %g", pop());
			return INTERPRET_OK;
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source)
{
	LOG_DEBUG("Compiling START");

	Chunk chunk;
	init_chunk(&chunk);

	if (!compile(source, &chunk)) {
		free_chunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	run();

	free_chunk(&chunk);

	LOG_DEBUG("Compiling END");

	return INTERPRET_OK;
}
