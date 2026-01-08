#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "log.h"
#include "value.h"
#include "vm.h"

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

__attribute__((format(printf, 1, 2))) static void
runtime_error(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fputs("\n", stderr);

	size_t instruction = (size_t)(vm.ip - vm.chunk->code - 1);

	size_t line = (size_t)vm.chunk->lines[instruction];

	fprintf(stderr, "[line %lu] in script\n", line);
	reset_stack();
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
		printf("[%d] ", i);
		print_value(*slot);
		putchar('\n');
	}
	LOG_TRACE("== Stack END ==");
	printf("\n");
}

static Value peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

static bool is_falsey(Value value)
{
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run(void)
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(TYPE, op)                                         \
	do {                                                        \
		if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {   \
			runtime_error("Operands must be numbers."); \
			return INTERPRET_RUNTIME_ERROR;             \
		}                                                   \
		double b = AS_NUMBER(pop());                        \
		double a = AS_NUMBER(pop());                        \
		push(TYPE(a op b));                                 \
	} while (false);

	Value result_value, top_value;
	uint8_t instruction;

	printf("\n");

	while (true) {
#ifdef DEBUG_TRACE_EXECUTION
		trace(true);
#endif

		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT:
			result_value = READ_CONSTANT();
			push(result_value);
			break;
		case OP_NIL:
			push(NIL_VAL);
			break;
		case OP_TRUE:
			push(BOOL_VAL(true));
			break;
		case OP_FALSE:
			push(BOOL_VAL(false));
			break;
		case OP_EQUAL: {
			Value b = pop();
			Value a = pop();
			push(BOOL_VAL(values_equal(a, b)));
			break;
		}
		case OP_GREATER:
			BINARY_OP(BOOL_VAL, >);
			break;
		case OP_LESS:
			BINARY_OP(BOOL_VAL, <);
			break;
		case OP_ADD:
			BINARY_OP(NUMBER_VAL, +);
			break;
		case OP_SUBTRACT:
			BINARY_OP(NUMBER_VAL, -);
			break;
		case OP_MULTIPLY:
			BINARY_OP(NUMBER_VAL, *);
			break;
		case OP_DIVIDE:
			BINARY_OP(NUMBER_VAL, /);
			break;

		// doing it in-place - no pop/push
		case OP_NOT:
		case OP_NEGATE: {
			top_value = *(vm.stack_top - 1);

			if (instruction == OP_NOT) {
				result_value = BOOL_VAL(is_falsey(top_value));
			} else {
				// for OP_NEGATE
				if (!IS_NUMBER(top_value)) {
					runtime_error(
						"Operand must be a number.");
					return INTERPRET_RUNTIME_ERROR;
				}
				result_value =
					NUMBER_VAL(-AS_NUMBER(top_value));
			}

			*(vm.stack_top - 1) = result_value;
			break;
		}
		case OP_RETURN:
			LOG_TRACE("RETURN");
			printf("\t->\t");
			print_value(pop());
			putchar('\n');
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
