#include <stddef.h>

#include "call.h"
#include "error.h"
#include "hash_table.h"
#include "internal.h"
#include "machinery.h"
#include "object.h"
#include "value.h"
#include "vm.h"

bool call(ClosureObject *closure, int arg_count)
{
	if ((size_t)arg_count != closure->function->arity) {
		Gustav_Runtime_Error("Expected %d arguments but got %d.",
				     (int)closure->function->arity, arg_count);
		return false;
	}

	if (vm.frame_count == FRAMES_MAX) {
		Gustav_Runtime_Error("Stack overflow."); // yes
		return false;
	}

	CallFrame *frame = &vm.frames[vm.frame_count++];
	frame->closure = closure;
	frame->ip = closure->function->chunk.code;
	frame->slots = vm.stack_top - arg_count - 1;

	return true;
}

bool call_value(Value callee, int arg_count)
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
			vm.stack_top[-arg_count - 1] = OBJ_VAL(Instance_New(klass));

			Value initializer;
			if (HashTable_GetItem(&klass->methods, vm.init_string, &initializer)) {
				return call(AS_CLOSURE(initializer), arg_count);
			}
			if (arg_count != 0) {
				Gustav_Runtime_Error("Expected 0 arguments but got %d.", arg_count);
				return false;
			}
			return true;
		}
		case OBJ_CLOSURE:
			return call(AS_CLOSURE(callee), arg_count);
		case OBJ_NATIVE: {
			// TODO(abduaziz): arity check, runtime errors ...
			NativeFn native = AS_NATIVE(callee);
			Value result = native(arg_count, vm.stack_top - arg_count);
			vm.stack_top -= arg_count + 1;
			VM_Push(result);
			return true;
		}
		default:
			break;
		}
	}

	Gustav_Runtime_Error("Can only call functions and classes.");
	return false;
}

bool invoke_from_class(ClassObject *klass, StringObject *name, int arg_count)
{
	Value method;

	if (!HashTable_GetItem(&klass->methods, name, &method)) {
		Gustav_Runtime_Error("Undefined property '%s'.", name->chars);
		return false;
	}

	return call(AS_CLOSURE(method), arg_count);
}

bool invoke(StringObject *name, int arg_count)
{
	Value receiver = VM_Peek(arg_count);

	if (!Instance_Check(receiver)) {
		Gustav_Runtime_Error("Only instance have methods.");
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

bool bind_method(ClassObject *klass, StringObject *name)
{
	Value method;

	if (!HashTable_GetItem(&klass->methods, name, &method)) {
		Gustav_Runtime_Error("Undefined property '%s'.", name->chars);
		return false;
	}

	BoundMethodObject *bound = BoundMethod_New(VM_Peek(0), AS_CLOSURE(method));

	VM_Pop();
	VM_Push(OBJ_VAL(bound));

	return true;
}

void define_method(StringObject *name)
{
	Value method = VM_Peek(0);
	ClassObject *klass = AS_CLASS(VM_Peek(1));
	HashTable_SetItem(&klass->methods, name, method);
	VM_Pop();
}
