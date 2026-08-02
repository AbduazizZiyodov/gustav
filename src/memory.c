#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "gc.h"
#include "hash_table.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

void *Mem_Realloc(void *pointer, size_t old_size, size_t new_size)
{
	if (new_size > old_size) {
		vm.bytes_allocated += new_size - old_size;
#ifdef DEBUG_STRESS_GC
		GC_Collect();
#endif
		if (vm.bytes_allocated > vm.next_gc) {
			GC_Collect();
		}
	} else {
		vm.bytes_allocated -= old_size - new_size;
	}

	if (new_size == 0) {
		free(pointer);
		return NULL;
	}

	void *result = realloc(pointer, new_size);

	if (result is NULL) {
		gustav_error(EXIT_FAILURE, "Can't perform reallocate");
	}

	return result;
}

void Mem_FreeObject(Object *object)
{
	ClosureObject *closure;

	switch (object->type) {
	case OBJ_BOUND_METHOD:
		FREE(BoundMethodObject, object);
		break;
	case OBJ_CLASS: {
		ClassObject *klass = (ClassObject *)object;
		HashTable_Free(&klass->methods);
		FREE(ClassObject, object);
		break;
	}
	case OBJ_INSTANCE: {
		InstanceObject *instance = (InstanceObject *)object;
		HashTable_Free(&instance->fields);
		FREE(InstanceObject, object);
		break;
	}
	case OBJ_STRING: {
		StringObject *string = (StringObject *)object;
		LOG_GC("Freeing string object: object=%p string_repr=%s\n",
		       object, string->chars);
		FREE_ARRAY(char, string->chars, string->length + 1);
		FREE(StringObject, object);
		break;
	}
	case OBJ_FUNCTION: {
		FunctionObject *function = (FunctionObject *)object;
		Chunk_Free(&function->chunk);
		FREE(FunctionObject, object);
		break;
	}
	case OBJ_NATIVE:
		FREE(NativeObject, object);
		break;
	case OBJ_CLOSURE:
		closure = (ClosureObject *)object;
		/*NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)*/
		FREE_ARRAY(UpvalueObject *, closure->upvalues,
			   closure->upvalue_count);
		FREE(ClosureObject, object);
		break;
	case OBJ_UPVALUE:
		FREE(UpvalueObject, object);
		break;
	}
}

void Mem_FreeObjects(void)
{
	Object *object = vm.objects;

	while (object != NULL) {
		Object *next = object->next;
		Mem_FreeObject(object);
		object = next;
	}

	free((void *)vm.gray_stack);
}
