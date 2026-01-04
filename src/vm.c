#include "common.h"
#include "vm.h"
#include "debug.h"

static VM vm;

/// Stack
static void reset_stack(void)
{
	vm.stack_top = vm.stack;
}

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

/// VM
void init_vm(void)
{
	LOG_INFO("VM was initialized");
	reset_stack();
}

void free_vm(void)
{
	LOG_INFO("VM was freed");
}

/// Interpretation
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

	LOG_INFO("Begin run()");
	Value constant;

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		LOG_TRACE("### STACK ###");
		int i = 0;
		for (Value *slot = vm.stack; slot < vm.stack_top; i++, slot++) {
			LOG_TRACE("[%d] %g", i, *slot);
		}
		printf("\n");
		disassemble_instruction(vm.chunk,
					(size_t)(vm.ip - vm.chunk->code));
#endif
		uint8_t instruction;

		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT:
			constant = READ_CONSTANT();
			push(constant);
			LOG_TRACE("CONSTANT=%g", constant);
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
			push(-pop());
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

InterpretResult interpret(Chunk *chunk)
{
	vm.chunk = chunk;
	vm.ip = vm.chunk->code;
	return run();
}
