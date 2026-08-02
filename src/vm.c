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
#include "native_functions.h"
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
#define READ_CONSTANT() \
	(frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(TYPE, op)                                             \
	do {                                                            \
		if (!Number_Check(peek(0)) || !Number_Check(peek(1))) { \
			runtime_error("Operands must be numbers.");     \
			return INTERPRET_RUNTIME_ERROR;                 \
		}                                                       \
		b = AS_NUMBER(VM_Pop());                                \
		a = AS_NUMBER(VM_Pop());                                \
		VM_Push(TYPE(a op b));                                  \
	} while (false);

void VM_Push(Value value)
{
	*vm.stack_top = value;
	vm.stack_top++;
}

Value VM_Pop(void)
{
	vm.stack_top--;
	return *vm.stack_top;
}

static void reset_stack(void)
{
	vm.stack_top = vm.stack;
	vm.frame_count = 0;
	vm.open_upvalues = NULL;
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
		CallFrame *frame = &vm.frames[i];
		FunctionObject *function = frame->closure->function;

		/*
		 * ip already points at the *next* instruction, so the one
		 * that faulted sits at ip - 1. A frame that has not executed
		 * anything yet (a native raising on entry) would make that
		 * negative and wrap, so clamp into the chunk before indexing.
		 */
		int line = 0;

		if (function->chunk.count > 0) {
			ptrdiff_t offset = frame->ip - function->chunk.code - 1;
			size_t instruction = offset < 0 ? 0 : (size_t)offset;

			if (instruction >= function->chunk.count) {
				instruction = function->chunk.count - 1;
			}

			line = function->chunk.lines[instruction];
		}

		(void)fprintf(stderr, "[at line %d] in ", line);

		if (function->name == NULL) {
			(void)fprintf(stderr, "script\n");
		} else {
			(void)fprintf(stderr, "%s()\n", function->name->chars);
		}
	}

	reset_stack();
}

static void define_native(const char *name, NativeFn function)
{
	VM_Push(OBJ_VAL(String_FromChars(name, strlen(name))));
	VM_Push(OBJ_VAL(Native_New(function)));
	HashTable_SetItem(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
	VM_Pop();
	VM_Pop();
}

void VM_Init(void)
{
	LOG_INFO("VM initialized\n");
	reset_stack();
	vm.objects = NULL;

	vm.bytes_allocated = 0;
	vm.next_gc = (size_t)(1024 * 1024);

	vm.gray_count = 0;
	vm.gray_capacity = 0;
	vm.gray_stack = NULL;

	HashTable_Init(&vm.strings);
	HashTable_Init(&vm.globals);

	vm.init_string = NULL;
	vm.init_string = String_FromChars("init", 4);

	for (size_t i = 0; i < NATIVE_FUNCTION_COUNT; i++) {
		NativeFunctionPair pair = NATIVE_FUNCTIONS[i];
		define_native(pair.name, pair.function);
	}
}

void VM_Free(void)
{
	LOG_DEBUG("Running cleanup ...\n");
	Mem_FreeObjects();
	HashTable_Free(&vm.strings);
	HashTable_Free(&vm.globals);
	vm.init_string = NULL;
	LOG_INFO("VM freed\n");
}

#ifdef DEBUG // DEBUG
static void trace(CallFrame *frame)
#else
static void trace(CallFrame *frame [[maybe_unused]])
#endif // DEBUG - mark arg as unused on release/non-debug builds
{
// Prints the instruction that currently being executed (if enabled) &
// content of the stack
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

static Value peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

static bool call(ClosureObject *closure, int arg_count)
{
	if ((size_t)arg_count != closure->function->arity) {
		runtime_error("Expected %d arguments but got %d.",
			      (int)closure->function->arity, arg_count);
		return false;
	}

	if (vm.frame_count == FRAMES_MAX) {
		runtime_error("Stack overflow."); // yes
		return false;
	}

	CallFrame *frame = &vm.frames[vm.frame_count++];
	frame->closure = closure;
	frame->ip = closure->function->chunk.code;
	frame->slots = vm.stack_top - arg_count - 1;

	return true;
}

static bool call_value(Value callee, int arg_count)
{
	if (Object_Check(callee)) {
		switch (OBJ_TYPE(callee)) {
		case OBJ_BOUND_METHOD: {
			BoundMethodObject *bound = AS_BOUND_METHOD(callee);
			vm.stack_top[-arg_count - 1] = bound->receiver;
			return call(bound->method, arg_count);
		}
		case OBJ_CLASS: {
			ClassObject *klass = AS_CLASS(callee);
			vm.stack_top[-arg_count - 1] =
				OBJ_VAL(Instance_New(klass));

			Value initializer;
			if (HashTable_GetItem(&klass->methods, vm.init_string,
					      &initializer)) {
				return call(AS_CLOSURE(initializer), arg_count);
			}
			if (arg_count != 0) {
				runtime_error(
					"Expected 0 arguments but got %d.",
					arg_count);
				return false;
			}
			return true;
		}
		case OBJ_CLOSURE:
			return call(AS_CLOSURE(callee), arg_count);
		case OBJ_NATIVE: {
			// TODO(abduaziz): arity check, runtime errors ...
			NativeFn native = AS_NATIVE(callee);
			Value result =
				native(arg_count, vm.stack_top - arg_count);
			vm.stack_top -= arg_count + 1;
			VM_Push(result);
			return true;
		}
		default:
			break;
		}
	}

	runtime_error("Can only call functions and classes.");
	return false;
}

static bool invoke_from_class(ClassObject *klass, StringObject *name,
			      int arg_count)
{
	Value method;

	if (!HashTable_GetItem(&klass->methods, name, &method)) {
		runtime_error("Undefined property '%s'.", name->chars);
		return false;
	}

	return call(AS_CLOSURE(method), arg_count);
}

static bool invoke(StringObject *name, int arg_count)
{
	Value receiver = peek(arg_count);

	if (!Instance_Check(receiver)) {
		runtime_error("Only instance have methods.");
		return false;
	}

	InstanceObject *instance = AS_INSTANCE(receiver);

	Value value;

	if (HashTable_GetItem(&instance->fields, name, &value)) {
		vm.stack_top[-arg_count - 1] = value;
		return call_value(value, arg_count);
	}

	return invoke_from_class(instance->klass, name, arg_count);
}

static bool bind_method(ClassObject *klass, StringObject *name)
{
	Value method;

	if (!HashTable_GetItem(&klass->methods, name, &method)) {
		runtime_error("Undefined property '%s'.", name->chars);
		return false;
	}

	BoundMethodObject *bound = BoundMethod_New(peek(0), AS_CLOSURE(method));

	VM_Pop();
	VM_Push(OBJ_VAL(bound));
	return true;
}

static UpvalueObject *capture_upvalue(Value *local)
{
	UpvalueObject *prev_upvalue = NULL;
	UpvalueObject *upvalue = vm.open_upvalues;

	while (upvalue != NULL && upvalue->location > local) {
		prev_upvalue = upvalue;
		upvalue = upvalue->next;
	}

	if (upvalue != NULL && upvalue->location == local) {
		return upvalue;
	}

	UpvalueObject *created_upvalue = Upvalue_New(local);

	created_upvalue->next = upvalue;

	if (prev_upvalue == NULL) {
		vm.open_upvalues = created_upvalue;
	} else {
		prev_upvalue->next = created_upvalue;
	}

	return created_upvalue;
}

static void close_upvalues(const Value *last)
{
	while (vm.open_upvalues != NULL && vm.open_upvalues->location >= last) {
		UpvalueObject *upvalue = vm.open_upvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;
		vm.open_upvalues = upvalue->next;
	}
}

static void define_method(StringObject *name)
{
	Value method = peek(0);
	ClassObject *klass = AS_CLASS(peek(1));
	LOG_TRACE("ADDING METHOD !!!!");
	HashTable_SetItem(&klass->methods, name, method);
	VM_Pop();
}

static bool is_falsey(Value value)
{
	return Nil_Check(value) || (Bool_Check(value) && !AS_BOOL(value));
}

static void concatenate(void)
{
	StringObject *b = AS_STRING(peek(0));
	StringObject *a = AS_STRING(peek(1));

	size_t total_length = a->length + b->length;

	char *chars = ALLOCATE(char, total_length + 1);

	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);

	chars[total_length] = '\0';

	StringObject *concatenated = String_FromOwnedChars(chars, total_length);

	VM_Pop();
	VM_Pop();
	VM_Push(OBJ_VAL(concatenated));
}

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
			frame->slots[slot] = peek(0);
			break;
		}
		case OP_GET_LOCAL: {
			uint8_t slot = READ_BYTE();
			Value slot_value = frame->slots[slot];

			if (Uninitialized_Check(slot_value)) {
				runtime_error(
					"Can't use uninitialized variable.");
				return INTERPRET_RUNTIME_ERROR;
			}

			VM_Push(slot_value);
			break;
		}
		case OP_DEFINE_GLOBAL: {
			StringObject *name = READ_STRING();
			HashTable_SetItem(&vm.globals, name, peek(0));
			VM_Pop();
			break;
		}
		case OP_GET_GLOBAL: {
			StringObject *name = READ_STRING();
			Value value;

			if (!HashTable_GetItem(&vm.globals, name, &value)) {
				runtime_error("Undefined variable '%s'.",
					      name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}

			if (Uninitialized_Check(value)) {
				runtime_error(
					"Can't use uninitialized variable '%s'.",
					name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}

			VM_Push(value);
			break;
		}
		case OP_SET_GLOBAL: {
			StringObject *name = READ_STRING();
			if (HashTable_SetItem(&vm.globals, name, peek(0))) {
				HashTable_DelItem(&vm.globals, name);
				runtime_error("Undefined variable '%s'.",
					      name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_GET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			Value slot_value =
				*frame->closure->upvalues[slot]->location;

			if (Uninitialized_Check(slot_value)) {
				runtime_error(
					"Can't use uninitialized variable.");
				return INTERPRET_RUNTIME_ERROR;
			}

			VM_Push(slot_value);
			break;
		}
		case OP_SET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			*frame->closure->upvalues[slot]->location = peek(0);
			break;
		}
		case OP_GET_PROPERTY: {
			if (!Instance_Check(peek(0))) {
				runtime_error(
					"Only instances have properties.");
				return INTERPRET_RUNTIME_ERROR;
			}

			InstanceObject *instance = AS_INSTANCE(peek(0));
			StringObject *name = READ_STRING();

			Value value;

			if (HashTable_GetItem(&instance->fields, name,
					      &value)) {
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
			if (!Instance_Check(peek(1))) {
				runtime_error("Only instances have fields.");
				return INTERPRET_RUNTIME_ERROR;
			}
			InstanceObject *instance = AS_INSTANCE(peek(1));
			HashTable_SetItem(&instance->fields, READ_STRING(),
					  peek(0));
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
			if (String_Check(peek(0)) && String_Check(peek(1))) {
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
			if (!Number_Check(peek(0)) || !Number_Check(peek(1))) {
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
					closure->upvalues[i] = capture_upvalue(
						frame->slots + index);
				} else {
					closure->upvalues[i] =
						frame->closure->upvalues[index];
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
		case OP_CLASS:
			VM_Push(OBJ_VAL(Class_New(READ_STRING())));
			break;
		case OP_INHERIT: {
			Value superclass = peek(1);
			if (!Class_Check(superclass)) {
				runtime_error("Superclass must be a class.");
				return INTERPRET_RUNTIME_ERROR;
			}
			ClassObject *subclass = AS_CLASS(peek(0));
			HashTable_AddAll(&AS_CLASS(superclass)->methods,
					 &subclass->methods);
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
