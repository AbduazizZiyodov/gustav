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
		if (!Number_Check(vm_peek(0)) || !Number_Check(vm_peek(1))) { \
			gustav_runtime_error("Operands must be numbers.");    \
			return INTERPRET_RUNTIME_ERROR;                       \
		}                                                             \
		b = AS_NUMBER(vm_pop());                                      \
		a = AS_NUMBER(vm_pop());                                      \
		vm_push(TYPE(a op b));                                        \
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

	LOG_TRACE("getting into VM switch loop\n");

	while (true) {
		vm_trace(frame);

		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT:
			result_value = READ_CONSTANT();
			vm_push(result_value);
			break;
		case OP_NIL:
			vm_push(NIL_VAL);
			break;
		case OP_TRUE:
			vm_push(BOOL_VAL(true));
			break;
		case OP_FALSE:
			vm_push(BOOL_VAL(false));
			break;
		case OP_UNINITIALIZED:
			vm_push(UNINITIALIZED_VAL);
			break;
		case OP_POP:
			vm_pop();
			break;
		case OP_SET_LOCAL: {
			uint8_t slot = READ_BYTE();
			frame->slots[slot] = vm_peek(0);
			break;
		}
		case OP_GET_LOCAL: {
			uint8_t slot = READ_BYTE();
			Value slot_value = frame->slots[slot];

			if (Uninitialized_Check(slot_value)) {
				gustav_runtime_error("Can't use uninitialized variable.");
				return INTERPRET_RUNTIME_ERROR;
			}

			vm_push(slot_value);
			break;
		}
		case OP_DEFINE_GLOBAL: {
			StringObject *name = READ_STRING();
			hash_table_set_item(&vm.globals, name, vm_peek(0));
			vm_pop();
			break;
		}
		case OP_GET_GLOBAL: {
			StringObject *name = READ_STRING();
			Value value;

			if (!hash_table_get_item(&vm.globals, name, &value)) {
				gustav_runtime_error("Undefined variable '%s'.", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}

			if (Uninitialized_Check(value)) {
				gustav_runtime_error("Can't use uninitialized variable '%s'.",
						     name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}

			vm_push(value);
			break;
		}
		case OP_SET_GLOBAL: {
			StringObject *name = READ_STRING();
			if (hash_table_set_item(&vm.globals, name, vm_peek(0))) {
				hash_table_delete_item(&vm.globals, name);
				gustav_runtime_error("Undefined variable '%s'.", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_GET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			Value slot_value = *frame->closure->upvalues[slot]->location;

			if (Uninitialized_Check(slot_value)) {
				gustav_runtime_error("Can't use uninitialized variable.");
				return INTERPRET_RUNTIME_ERROR;
			}

			vm_push(slot_value);
			break;
		}
		case OP_SET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			*frame->closure->upvalues[slot]->location = vm_peek(0);
			break;
		}
		case OP_GET_PROPERTY: {
			if (!Instance_Check(vm_peek(0))) {
				gustav_runtime_error("Only instances have properties.");
				return INTERPRET_RUNTIME_ERROR;
			}

			InstanceObject *instance = AS_INSTANCE(vm_peek(0));
			StringObject *name = READ_STRING();

			Value value;

			if (hash_table_get_item(&instance->fields, name, &value)) {
				vm_pop();
				vm_push(value);
				break;
			}

			if (!vm_bind_method(instance->klass, name)) {
				return INTERPRET_RUNTIME_ERROR;
			}

			break;
		}
		case OP_SET_PROPERTY: {
			if (!Instance_Check(vm_peek(1))) {
				gustav_runtime_error("Only instances have fields.");
				return INTERPRET_RUNTIME_ERROR;
			}
			InstanceObject *instance = AS_INSTANCE(vm_peek(1));
			hash_table_set_item(&instance->fields, READ_STRING(), vm_peek(0));
			Value value = vm_pop();
			vm_pop();
			vm_push(value);
			break;
		}
		case OP_EQUAL: {
			y = vm_pop();
			x = vm_pop();
			result_value = BOOL_VAL(value_equal(x, y));
			vm_push(result_value);
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
			if (String_Check(vm_peek(0)) && String_Check(vm_peek(1))) {
				vm_string_concatenate();
			} else {
				gustav_runtime_error(
					"Operands must be two strings to concatenate.");
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
			if (!Number_Check(vm_peek(0)) || !Number_Check(vm_peek(1))) {
				gustav_runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			b = AS_NUMBER(vm_pop());
			a = AS_NUMBER(vm_pop());
			vm_push(NUMBER_VAL(pow(a, b)));
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
					gustav_runtime_error("Operand must be a number.");
					return INTERPRET_RUNTIME_ERROR;
				}
				result_value = NUMBER_VAL(-AS_NUMBER(top_value));
			}

			*(vm.stack_top - 1) = result_value;
			break;
		}
		case OP_PRINT_STDOUT: {
			LOG_DEBUG("== [stdout] ==\n");
			print_value(stdout, vm_pop());
			fputc('\n', stdout);
			fflush(stdout); // NOTE(Abduaziz): I had hard time to find out its buffered
			LOG_DEBUG("== [/stdout] ==\n\n");
			break;
		}
		case OP_PRINT_STDERR: {
			LOG_DEBUG("== [stderr] ==\n");
			print_value(stderr, vm_pop());
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
			if (is_falsey(vm_peek(0))) {
				frame->ip += offset;
			}
			break;
		}
		case OP_LOOP: {
			uint16_t offset = READ_SHORT();
			frame->ip -= offset;
			break;
		}
		case OP_BREAK: {
			LOG_TRACE("op break is read\n");
			break;
		}
		case OP_CALL: {
			int arg_count = READ_BYTE();
			if (!vm_call_value(vm_peek(arg_count), arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		case OP_INVOKE: {
			StringObject *method = READ_STRING();
			int arg_count = READ_BYTE();

			if (!vm_invoke(method, arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		case OP_CLOSURE: {
			FunctionObject *function = AS_FUNCTION(READ_CONSTANT());
			ClosureObject *closure = new_closure(function);
			vm_push(OBJ_VAL(closure));

			for (int i = 0; i < closure->upvalue_count; i++) {
				uint8_t is_local = READ_BYTE();
				uint8_t index = READ_BYTE();

				if (is_local) {
					closure->upvalues[i] =
						vm_capture_upvalue(frame->slots + index);
				} else {
					closure->upvalues[i] = frame->closure->upvalues[index];
				}
			}
			break;
		}
		case OP_CLOSE_UPVALUE: {
			vm_close_upvalues(vm.stack_top - 1);
			vm_pop();
			break;
		}
		case OP_RETURN: {
			result_value = vm_pop();
			vm_close_upvalues(frame->slots);
			vm.frame_count--;
			if (vm.frame_count == 0) {
				vm_pop();
				return INTERPRET_OK;
			}
			vm.stack_top = frame->slots;
			vm_push(result_value);
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}
		case OP_RETURN_EXIT: {
			result_value = vm_pop();

			if (!Number_Check(result_value)) {
				gustav_runtime_error("Exit status must be a number.");
				return INTERPRET_RUNTIME_ERROR;
			}

			double status = AS_NUMBER(result_value);

			if (!isfinite(status)) {
				gustav_runtime_error("Exit status must be a finite number.");
				return INTERPRET_RUNTIME_ERROR;
			}

			vm.exit_status = ((int)fmod(status, 256.0)) & 0xff;

			return INTERPRET_EXIT;
		}
		case OP_CLASS:
			vm_push(OBJ_VAL(new_class(READ_STRING())));
			break;
		case OP_INHERIT: {
			Value superclass = vm_peek(1);

			if (!Class_Check(superclass)) {
				gustav_runtime_error("Superclass must be a class.");
				return INTERPRET_RUNTIME_ERROR;
			}

			ClassObject *subclass = AS_CLASS(vm_peek(0));
			hash_table_add_all(&AS_CLASS(superclass)->methods, &subclass->methods);
			vm_pop(); // subclass

			break;
		}
		case OP_GET_SUPER: {
			StringObject *name = READ_STRING();
			ClassObject *superclass = AS_CLASS(vm_pop());

			if (!vm_bind_method(superclass, name)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_METHOD:
			vm_define_method(READ_STRING());
			break;
		case OP_SUPER_INVOKE: {
			StringObject *method = READ_STRING();
			int arg_count = READ_BYTE();
			ClassObject *superclass = AS_CLASS(vm_pop());
			if (!vm_invoke_from_class(superclass, method, arg_count)) {
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
	LOG_DEBUG("\n\n== [source] ==\n%s\n== [/source] ==\n\n\n", source);

	LOG_DEBUG("== [compile] ==\n\n");
	FunctionObject *function = compile(source);
	LOG_DEBUG("== [/compile] ==\n\n");

	if (function == NULL) {
		return INTERPRET_COMPILE_ERROR;
	}

	LOG_TRACE("== [vm setup] ==\n");
	vm_push(OBJ_VAL(function));
	ClosureObject *closure = new_closure(function);
	vm_pop();
	vm_push(OBJ_VAL(closure));
	vm_call(closure, 0);
	LOG_TRACE("== [/vm setup] ==\n\n");

	return run();
}
