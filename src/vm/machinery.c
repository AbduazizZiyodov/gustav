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

Value VM_Peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

bool is_falsey(Value value)
{
	return Nil_Check(value) || (Bool_Check(value) && !AS_BOOL(value));
}

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

void VM_Reset_Stack(void)
{
	vm.stack_top = vm.stack;
	vm.frame_count = 0;
	vm.open_upvalues = NULL;
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
	VM_Reset_Stack();

	GC_Init();
	String_InitInterned();

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
	String_FreeInterned();
	HashTable_Free(&vm.globals);
	vm.init_string = NULL;
	LOG_INFO("VM freed\n");
}

int VM_ExitStatus(void)
{
	return vm.exit_status;
}

void VM_MarkRoots(void)
{
	for (Value *slot = vm.stack; slot < vm.stack_top; slot++) {
		GC_MarkValue(*slot);
	}

	for (size_t i = 0; i < vm.frame_count; i++) {
		GC_MarkObject((Object *)vm.frames[i].closure);
	}

	for (UpvalueObject *upvalue = vm.open_upvalues; upvalue != NULL; upvalue = upvalue->next) {
		GC_MarkObject((Object *)upvalue);
	}

	HashTable_Mark(&vm.globals);
	GC_MarkObject((Object *)vm.init_string);
}

void VM_String_Concatenate(void)
{
	StringObject *b = AS_STRING(VM_Peek(0));
	StringObject *a = AS_STRING(VM_Peek(1));

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
