#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#if DEBUG
#include "debug.h"
#endif

VM vm;

#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(TYPE, op)                                         \
	do {                                                        \
		if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {   \
			runtime_error("Operands must be numbers."); \
			return INTERPRET_RUNTIME_ERROR;             \
		}                                                   \
		b = AS_NUMBER(pop());                               \
		a = AS_NUMBER(pop());                               \
		push(TYPE(a op b));                                 \
	} while (false);

void push(value_t value)
{
	*vm.stack_top = value;
	vm.stack_top++;
}

value_t pop(void)
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
	(void)vfprintf(stderr, format, args);
	va_end(args);

	(void)fputs("\n", stderr);

	size_t instruction = (size_t)(vm.ip - vm.chunk->code - 1);

	size_t line = (size_t)vm.chunk->lines[instruction];

	(void)fprintf(stderr, "[line %lu] in script\n", line);
	reset_stack();
}

void init_vm(void)
{
	LOG_INFO("VM initialized\n");
	reset_stack();
	vm.objects = NULL;
}

void free_vm(void)
{
	LOG_TRACE("Running cleanup ...\n");
	free_objects();
	LOG_INFO("VM freed\n");
}

static void trace(void)
{
	// Prints the instruction that currently being executed (if enabled) &
	// content of the stack
#ifdef DEBUG
	size_t offset = (size_t)(vm.ip - vm.chunk->code);
	disassemble_instruction(vm.chunk, offset);

	LOG_TRACE("== Stack ==\n");
	uint16_t i = 0;

	for (value_t *slot = vm.stack; slot < vm.stack_top; i++, slot++) {
		printf("[%d] ", i);
		print_value(*slot);
		(void)putchar('\n');
	}

	LOG_TRACE("== Stack END ==\n");
	printf("\n");
#endif
}

static value_t peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

static bool is_falsey(value_t value)
{
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(void)
{
	obj_string_t *b = AS_STRING(pop());
	obj_string_t *a = AS_STRING(pop());

	size_t total_length = a->length + b->length; // 1 for \0

	char *chars = ALLOCATE(char, total_length + 1);

	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);

	chars[total_length] = '\0';

	obj_string_t *concatenated = take_string(chars, total_length);

	push(OBJ_VAL(concatenated));
}

static interpreter_result_t run(void)
{
	value_t result_value;
	value_t top_value;
	value_t x;
	value_t y;
	uint8_t instruction;
	double a;
	double b;

	while (true) {
		trace();

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
			y = pop();
			x = pop();
			result_value = BOOL_VAL(values_equal(x, y));
			push(result_value);
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
		case OP_CONCAT: {
			if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
				concatenate();
			} else {
				runtime_error(
					"Operands must be two strings to concatenate.");
			}
			break;
		}

		case OP_SUBTRACT:
			BINARY_OP(NUMBER_VAL, -);
			break;
		case OP_MULTIPLY:
			BINARY_OP(NUMBER_VAL, *);
			break;
		case OP_POW:
			if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			b = AS_NUMBER(pop());
			a = AS_NUMBER(pop());
			push(NUMBER_VAL(pow(a, b)));
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
			LOG_TRACE("RETURN => ");
			print_value(pop());
			(void)putchar('\n');
			return INTERPRET_OK;
		default:
			UNREACHABLE();
		}
	}
}

interpreter_result_t interpret(const char *source)
{
	LOG_DEBUG("Compiling START\n");

	chunk_t chunk;
	init_chunk(&chunk);

	if (!compile(source, &chunk)) {
		free_chunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	run();

	free_chunk(&chunk);

	LOG_DEBUG("Compiling END\n");

	return INTERPRET_OK;
}
