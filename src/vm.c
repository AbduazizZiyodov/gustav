#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "hash_table.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#if DEBUG
#include "debug.h"
#endif

VM vm;

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
	(frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
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
	vm.frame_count = 0;
}

__attribute__((format(printf, 1, 2))) static void
runtime_error(const char *format, ...) // TODO(abduaziz): better stack trace
{
	va_list args;

	va_start(args, format);
	(void)vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	for (int i = (int)vm.frame_count - 1; i >= 0; i--) {
		call_frame_t *frame = &vm.frames[i];
		function_t *function = frame->function;

		size_t instruction =
			(size_t)(frame->ip - function->chunk.code - 1);

		int line = frame->function->chunk.lines[instruction];

		(void)fprintf(stderr, "[line %d] in ", line);

		if (function->name == NULL) {
			(void)fprintf(stderr, "script\n");
		} else {
			(void)fprintf(stderr, "%s()\n", function->name->chars);
		}
	}

	reset_stack();
}

void init_vm(void)
{
	LOG_INFO("VM initialized\n");
	reset_stack();
	vm.objects = NULL;
	init_hash_table(&vm.strings);
	init_hash_table(&vm.globals);
}

void free_vm(void)
{
	LOG_DEBUG("Running cleanup ...\n");
	free_objects();
	free_hash_table(&vm.strings);
	free_hash_table(&vm.globals);
	LOG_INFO("VM freed\n");
}

static void trace(call_frame_t *frame)
{
// Prints the instruction that currently being executed (if enabled) &
// content of the stack
#ifdef DEBUG
	size_t offset = (size_t)(frame->ip - frame->function->chunk.code);
	disassemble_instruction(&frame->function->chunk, offset);

	LOG_DEBUG("== [stack] ==\n");
	uint16_t i = 0;

	for (value_t *slot = vm.stack; slot < vm.stack_top; i++, slot++) {
		printf("[%d] ", i);
		print_value(*slot);
		(void)putchar('\n');
	}

	LOG_DEBUG("== [/stack] ==\n");
	printf("\n");
#endif // DEBUG
}

static value_t peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

static bool call(function_t *function, int arg_count)
{
	if (arg_count != (int)function->arity) {
		runtime_error("Expected %d arguments but got %d",
			      (int)function->arity, arg_count);
		return false;
	}

	if (vm.frame_count == FRAMES_MAX) {
		runtime_error("Stack overflow."); // yes
		return false;
	}

	call_frame_t *frame = &vm.frames[vm.frame_count++];

	frame->function = function;
	frame->ip = function->chunk.code;
	frame->slots = vm.stack_top - arg_count - 1;

	return true;
}

static bool call_value(value_t callee, int arg_count)
{
	if (IS_OBJ(callee)) {
		switch (OBJ_TYPE(callee)) {
		case OBJ_FUNCTION:
			return call(AS_FUNCTION(callee), arg_count);
		default:
			break;
		}
	}

	runtime_error("Can only call functions and classes.");
	return false;
}

static bool is_falsey(value_t value)
{
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(void)
{
	string_t *b = AS_STRING(pop());
	string_t *a = AS_STRING(pop());

	size_t total_length = a->length + b->length;

	char *chars = ALLOCATE(char, total_length + 1);

	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);

	chars[total_length] = '\0';

	string_t *concatenated = take_string(chars, total_length);

	push(OBJ_VAL(concatenated));
}

/* NOLINTNEXTLINE(readability-function-size) */
static interpreter_result_t run(void)
{
	value_t result_value;
	value_t top_value;
	value_t x;
	value_t y;
	uint8_t instruction;
	double a;
	double b;

	call_frame_t *frame = &vm.frames[vm.frame_count - 1];

	while (true) {
		trace(frame);

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
		case OP_POP:
			pop();
			break;
		case OP_SET_LOCAL: {
			uint8_t slot = READ_BYTE();
			frame->slots[slot] = peek(0);
			break;
		}
		case OP_GET_LOCAL: {
			uint8_t slot = READ_BYTE();
			push(frame->slots[slot]);
			break;
		}
		case OP_DEFINE_GLOBAL: {
			string_t *name = READ_STRING();
			ht_insert(&vm.globals, name, peek(0));
			pop();
			break;
		}
		case OP_GET_GLOBAL: {
			string_t *name = READ_STRING();
			value_t value;

			if (!ht_get(&vm.globals, name, &value)) {
				runtime_error("Undefined variable '%s'.",
					      name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			push(value);
			break;
		}
		case OP_SET_GLOBAL: {
			string_t *name = READ_STRING();
			if (ht_insert(&vm.globals, name, peek(0))) {
				ht_delete(&vm.globals, name);
				runtime_error("Undefined variable '%s'.",
					      name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
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
		case OP_PRINT: {
			LOG_TRACE("== [stdout] ==\n");
			print_value(pop());
			(void)putchar('\n');
			LOG_TRACE("== [/stdout] ==\n\n");
			break;
		}
		case OP_JUMP: {
			uint16_t offset = READ_SHORT();
			frame->ip += offset;
			break;
		}
		case OP_JUMP_IF_FALSE: {
			uint16_t offset = READ_SHORT();
			if (is_falsey(peek(0))) {
				frame->ip += offset;
			}
			break;
		}
		case OP_LOOP: {
			uint16_t offset = READ_SHORT();
			frame->ip -= offset;
			break;
		}
		case OP_CALL: {
			int arg_count = READ_BYTE();
			if (!call_value(peek(arg_count), arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		case OP_RETURN:
			result_value = pop();
			vm.frame_count--;

			if (vm.frame_count == 0) {
				pop();
				return INTERPRET_OK;
			}

			vm.stack_top = frame->slots;
			push(result_value);
			frame = &vm.frames[vm.frame_count - 1];
			break;
		default:
			UNREACHABLE();
		}
	}
}

interpreter_result_t interpret(const char *source)
{
	LOG_DEBUG("\n== [source] ==\n%s\n== [/source] ==\n\n", source);
	LOG_INFO("Begin compiling\n");

	function_t *function = compile(source);
	if (function == NULL) {
		return INTERPRET_COMPILE_ERROR;
	}

	push(OBJ_VAL(function));
	call(function, 0);

	return run();
}
