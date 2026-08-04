#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "hash_table.h"
#include "internal.h"
#include "log.h"
#include "machinery.h"
#include "memory.h"
#include "native_functions.h"
#include "object.h"
#include "value.h"
#include "vm.h"

Value vm_peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

void vm_push(Value value)
{
	*vm.stack_top = value;
	vm.stack_top++;
}

Value vm_pop(void)
{
	vm.stack_top--;
	return *vm.stack_top;
}

void vm_reset_stack(void)
{
	vm.stack_top = vm.stack;
	vm.frame_count = 0;
	vm.open_upvalues = NULL;
}

bool is_falsey(Value value)
{
	return Nil_Check(value) || (Bool_Check(value) && !AS_BOOL(value));
}

static void define_native(const char *name, NativeFn function)
{
	vm_push(OBJ_VAL(string_from_chars(name, strlen(name))));
	vm_push(OBJ_VAL(new_native(function)));
	hash_table_set_item(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
	vm_pop();
	vm_pop();
}

void init_vm(void)
{
	LOG_INFO("VM initialized\n");
	vm_reset_stack();

	gc_init();
	init_interned_strings();

	init_hash_table(&vm.globals);

	vm.init_string = NULL;
	vm.init_string = string_from_chars("init", 4);

	for (size_t i = 0; i < NATIVE_FUNCTION_COUNT; i++) {
		NativeFunctionPair pair = NATIVE_FUNCTIONS[i];
		define_native(pair.name, pair.function);
	}
}

void free_vm(void)
{
	LOG_DEBUG("Running cleanup ...\n");
	mem_free_objects();
	free_interned_strings();
	free_tash_table(&vm.globals);
	vm.init_string = NULL;
	LOG_INFO("VM freed\n");
}

int vm_exit_status(void)
{
	return vm.exit_status;
}

void vm_mark_roots(void)
{
	for (Value *slot = vm.stack; slot < vm.stack_top; slot++) {
		gc_mark_value(*slot);
	}

	for (size_t i = 0; i < vm.frame_count; i++) {
		gc_mark_object((Object *)vm.frames[i].closure);
	}

	for (UpvalueObject *upvalue = vm.open_upvalues; upvalue != NULL; upvalue = upvalue->next) {
		gc_mark_object((Object *)upvalue);
	}

	hash_table_mark(&vm.globals);
	gc_mark_object((Object *)vm.init_string);
}

void vm_string_concatenate(void)
{
	StringObject *b = AS_STRING(vm_peek(0));
	StringObject *a = AS_STRING(vm_peek(1));

	size_t total_length = a->length + b->length;

	char *chars = ALLOCATE(char, total_length + 1);

	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);

	chars[total_length] = '\0';

	StringObject *concatenated = string_from_owned_chars(chars, total_length);

	vm_pop();
	vm_pop();
	vm_push(OBJ_VAL(concatenated));
}
