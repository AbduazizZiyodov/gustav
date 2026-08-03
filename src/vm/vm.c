#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "call.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "error.h"
#include "hash_table.h"
#include "internal.h"
#include "log.h"
#include "machinery.h"
#include "object.h"
#include "trace.h"
#include "upvalue.h"
#include "value.h"
#include "vm.h"

VM vm;

#define READ_BYTE() (*frame->ip++)

#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(TYPE, op)                                                   \
	do {                                                                  \
		if (!Number_Check(VM_Peek(0)) || !Number_Check(VM_Peek(1))) { \
			runtime_error("Operands must be numbers.");           \
			return INTERPRET_RUNTIME_ERROR;                       \
		}                                                             \
		b = AS_NUMBER(VM_Pop());                                      \
		a = AS_NUMBER(VM_Pop());                                      \
		VM_Push(TYPE(a op b));                                        \
	} while (false);

/* NOLINTNEXTLINE(readability-function-size) */
static InterpretResult run(void)
{
	Value result_value;
	Value top_value;
	Value x;
	Value y;
	uint8_t instruction;
	double a;
	double b;

	CallFrame *frame = &vm.frames[vm.frame_count - 1];

	while (true) {
		trace(frame);

		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT:
			result_value = READ_CONSTANT();
			VM_Push(result_value);
			break;
		case OP_NIL:
			VM_Push(NIL_VAL);
			break;
		case OP_TRUE:
			VM_Push(BOOL_VAL(true));
			break;
		case OP_FALSE:
			VM_Push(BOOL_VAL(false));
			break;
		case OP_UNINITIALIZED:
			VM_Push(UNINITIALIZED_VAL);
			break;
		case OP_POP:
			VM_Pop();
			break;
		case OP_SET_LOCAL: {
			uint8_t slot = READ_BYTE();
			frame->slots[slot] = VM_Peek(0);
			break;
		}
		case OP_GET_LOCAL: {
			uint8_t slot = READ_BYTE();
			Value slot_value = frame->slots[slot];

			if (Uninitialized_Check(slot_value)) {
				runtime_error("Can't use uninitialized variable.");
				return INTERPRET_RUNTIME_ERROR;
			}

			VM_Push(slot_value);
			break;
		}
		case OP_DEFINE_GLOBAL: {
			StringObject *name = READ_STRING();
			HashTable_SetItem(&vm.globals, name, VM_Peek(0));
			VM_Pop();
			break;
		}
		case OP_GET_GLOBAL: {
			StringObject *name = READ_STRING();
			Value value;

			if (!HashTable_GetItem(&vm.globals, name, &value)) {
				runtime_error("Undefined variable '%s'.", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}

			if (Uninitialized_Check(value)) {
				runtime_error("Can't use uninitialized variable '%s'.",
					      name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}

			VM_Push(value);
			break;
		}
		case OP_SET_GLOBAL: {
			StringObject *name = READ_STRING();
			if (HashTable_SetItem(&vm.globals, name, VM_Peek(0))) {
				HashTable_DelItem(&vm.globals, name);
				runtime_error("Undefined variable '%s'.", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_GET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			Value slot_value = *frame->closure->upvalues[slot]->location;

			if (Uninitialized_Check(slot_value)) {
				runtime_error("Can't use uninitialized variable.");
				return INTERPRET_RUNTIME_ERROR;
			}

			VM_Push(slot_value);
			break;
		}
		case OP_SET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			*frame->closure->upvalues[slot]->location = VM_Peek(0);
			break;
		}
		case OP_GET_PROPERTY: {
			if (!Instance_Check(VM_Peek(0))) {
				runtime_error("Only instances have properties.");
				return INTERPRET_RUNTIME_ERROR;
			}

			InstanceObject *instance = AS_INSTANCE(VM_Peek(0));
			StringObject *name = READ_STRING();

			Value value;

			if (HashTable_GetItem(&instance->fields, name, &value)) {
				VM_Pop();
				VM_Push(value);
				break;
			}

			if (!bind_method(instance->klass, name)) {
				return INTERPRET_RUNTIME_ERROR;
			}

			break;
		}
		case OP_SET_PROPERTY: {
			if (!Instance_Check(VM_Peek(1))) {
				runtime_error("Only instances have fields.");
				return INTERPRET_RUNTIME_ERROR;
			}
			InstanceObject *instance = AS_INSTANCE(VM_Peek(1));
			HashTable_SetItem(&instance->fields, READ_STRING(), VM_Peek(0));
			Value value = VM_Pop();
			VM_Pop();
			VM_Push(value);
			break;
		}
		case OP_EQUAL: {
			y = VM_Pop();
			x = VM_Pop();
			result_value = BOOL_VAL(Value_Equal(x, y));
			VM_Push(result_value);
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
			if (String_Check(VM_Peek(0)) && String_Check(VM_Peek(1))) {
				concatenate();
			} else {
				runtime_error("Operands must be two strings to concatenate.");
				return INTERPRET_RUNTIME_ERROR;
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
			if (!Number_Check(VM_Peek(0)) || !Number_Check(VM_Peek(1))) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			b = AS_NUMBER(VM_Pop());
			a = AS_NUMBER(VM_Pop());
			VM_Push(NUMBER_VAL(pow(a, b)));
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
				if (!Number_Check(top_value)) {
					runtime_error("Operand must be a number.");
					return INTERPRET_RUNTIME_ERROR;
				}
				result_value = NUMBER_VAL(-AS_NUMBER(top_value));
			}

			*(vm.stack_top - 1) = result_value;
			break;
		}
		case OP_PRINT_STDOUT: {
			LOG_DEBUG("== [stdout] ==\n");
			Value_Print(stdout, VM_Pop());
			fputc('\n', stdout);
			fflush(stdout); // NOTE(abduaziz): I had hard time to find out its buffered
			LOG_DEBUG("== [/stdout] ==\n\n");
			break;
		}
		case OP_PRINT_STDERR: {
			LOG_DEBUG("== [stderr] ==\n");
			Value_Print(stderr, VM_Pop());
			(void)fputc('\n', stderr);
			fflush(stderr);
			LOG_DEBUG("== [/stderr] ==\n\n");
			break;
		}
		case OP_JUMP: {
			uint16_t offset = READ_SHORT();
			frame->ip += offset;
			break;
		}
		case OP_JUMP_IF_FALSE: {
			uint16_t offset = READ_SHORT();
			if (is_falsey(VM_Peek(0))) {
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
			if (!call_value(VM_Peek(arg_count), arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		case OP_INVOKE: {
			StringObject *method = READ_STRING();
			int arg_count = READ_BYTE();

			if (!invoke(method, arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		case OP_CLOSURE: {
			FunctionObject *function = AS_FUNCTION(READ_CONSTANT());
			ClosureObject *closure = Closure_New(function);
			VM_Push(OBJ_VAL(closure));

			for (int i = 0; i < closure->upvalue_count; i++) {
				uint8_t is_local = READ_BYTE();
				uint8_t index = READ_BYTE();

				if (is_local) {
					closure->upvalues[i] =
						capture_upvalue(frame->slots + index);
				} else {
					closure->upvalues[i] = frame->closure->upvalues[index];
				}
			}
			break;
		}
		case OP_CLOSE_UPVALUE: {
			close_upvalues(vm.stack_top - 1);
			VM_Pop();
			break;
		}
		case OP_RETURN: {
			result_value = VM_Pop();
			close_upvalues(frame->slots);
			vm.frame_count--;
			if (vm.frame_count == 0) {
				VM_Pop();
				return INTERPRET_OK;
			}
			vm.stack_top = frame->slots;
			VM_Push(result_value);
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		case OP_RETURN_EXIT: {
			result_value = VM_Pop();

			if (!Number_Check(result_value)) {
				runtime_error("Exit status must be a number.");
				return INTERPRET_RUNTIME_ERROR;
			}

			double status = AS_NUMBER(result_value);

			if (!isfinite(status)) {
				runtime_error("Exit status must be a finite number.");
				return INTERPRET_RUNTIME_ERROR;
			}

			vm.exit_status = ((int)fmod(status, 256.0)) & 0xff;

			return INTERPRET_EXIT;
		}
		case OP_CLASS:
			VM_Push(OBJ_VAL(Class_New(READ_STRING())));
			break;
		case OP_INHERIT: {
			Value superclass = VM_Peek(1);

			if (!Class_Check(superclass)) {
				runtime_error("Superclass must be a class.");
				return INTERPRET_RUNTIME_ERROR;
			}

			ClassObject *subclass = AS_CLASS(VM_Peek(0));
			HashTable_AddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
			VM_Pop(); // subclass

			break;
		}
		case OP_GET_SUPER: {
			StringObject *name = READ_STRING();
			ClassObject *superclass = AS_CLASS(VM_Pop());

			if (!bind_method(superclass, name)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_METHOD:
			define_method(READ_STRING());
			break;
		case OP_SUPER_INVOKE: {
			StringObject *method = READ_STRING();
			int arg_count = READ_BYTE();
			ClassObject *superclass = AS_CLASS(VM_Pop());
			if (!invoke_from_class(superclass, method, arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		default:
			UNREACHABLE();
		}
	}
}

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

InterpretResult VM_Interpret(const char *source)
{
	LOG_DEBUG("\n== [source] ==\n%s\n== [/source] ==\n\n", source);
	LOG_INFO("Begin compiling\n");

	FunctionObject *function = Compiler_Compile(source);
	if (function == NULL) {
		return INTERPRET_COMPILE_ERROR;
	}

	VM_Push(OBJ_VAL(function));

	ClosureObject *closure = Closure_New(function);
	VM_Pop();
	VM_Push(OBJ_VAL(closure));
	call(closure, 0);

	return run();
}
