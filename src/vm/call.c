#include <stddef.h>

#include "call.h"
#include "error.h"
#include "hash_table.h"
#include "internal.h"
#include "machinery.h"
#include "object.h"
#include "value.h"
#include "vm.h"

bool vm_call(ClosureObject *closure, int arg_count)
{
	if ((size_t)arg_count != closure->function->arity) {
		gustav_runtime_error("Expected %d arguments but got %d.",
				     (int)closure->function->arity, arg_count);
		return false;
	}

	if (vm.frame_count == FRAMES_MAX) {
		gustav_runtime_error("Stack overflow."); // yes
		return false;
	}

	CallFrame *frame = &vm.frames[vm.frame_count++];
	frame->closure = closure;
	frame->ip = closure->function->chunk.code;
	frame->slots = vm.stack_top - arg_count - 1;

	return true;
}

bool vm_call_value(Value callee, int arg_count)
{
	if (Object_Check(callee)) {
		switch (OBJ_TYPE(callee)) {
		case OBJ_BOUND_METHOD: {
			BoundMethodObject *bound = AS_BOUND_METHOD(callee);
			vm.stack_top[-arg_count - 1] = bound->receiver;
			return vm_call(bound->method, arg_count);
		}
		case OBJ_CLASS: {
			ClassObject *klass = AS_CLASS(callee);
			vm.stack_top[-arg_count - 1] = OBJ_VAL(new_instance(klass));

			Value initializer;
			if (hash_table_get_item(&klass->methods, vm.init_string, &initializer)) {
				return vm_call(AS_CLOSURE(initializer), arg_count);
			}
			if (arg_count != 0) {
				gustav_runtime_error("Expected 0 arguments but got %d.", arg_count);
				return false;
			}
			return true;
		}
		case OBJ_CLOSURE:
			return vm_call(AS_CLOSURE(callee), arg_count);
		case OBJ_NATIVE: {
			// TODO(abduaziz): arity check, runtime errors ...
			NativeFn native = AS_NATIVE(callee);
			Value result = native(arg_count, vm.stack_top - arg_count);
			vm.stack_top -= arg_count + 1;
			vm_push(result);
			return true;
		}
		default:
			break;
		}
	}

	gustav_runtime_error("Can only call functions and classes.");
	return false;
}

bool vm_invoke_from_class(ClassObject *klass, StringObject *name, int arg_count)
{
	Value method;

	if (!hash_table_get_item(&klass->methods, name, &method)) {
		gustav_runtime_error("Undefined property '%s'.", name->chars);
		return false;
	}

	return vm_call(AS_CLOSURE(method), arg_count);
}

bool vm_invoke(StringObject *name, int arg_count)
{
	Value receiver = vm_peek(arg_count);

	if (!Instance_Check(receiver)) {
		gustav_runtime_error("Only instance have methods.");
		return false;
	}

	InstanceObject *instance = AS_INSTANCE(receiver);

	Value value;

	if (hash_table_get_item(&instance->fields, name, &value)) {
		vm.stack_top[-arg_count - 1] = value;
		return vm_call_value(value, arg_count);
	}

	return vm_invoke_from_class(instance->klass, name, arg_count);
}

bool vm_bind_method(ClassObject *klass, StringObject *name)
{
	Value method;

	if (!hash_table_get_item(&klass->methods, name, &method)) {
		gustav_runtime_error("Undefined property '%s'.", name->chars);
		return false;
	}

	BoundMethodObject *bound = new_bound_method(vm_peek(0), AS_CLOSURE(method));

	vm_pop();
	vm_push(OBJ_VAL(bound));

	return true;
}

void vm_define_method(StringObject *name)
{
	Value method = vm_peek(0);
	ClassObject *klass = AS_CLASS(vm_peek(1));
	hash_table_set_item(&klass->methods, name, method);
	vm_pop();
}
