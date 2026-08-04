#include <stdlib.h>

#include "internal.h"
#include "object.h"
#include "upvalue.h"
#include "value.h"

UpvalueObject *vm_capture_upvalue(Value *local)
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

	UpvalueObject *created_upvalue = new_upvalue(local);

	created_upvalue->next = upvalue;

	if (prev_upvalue == NULL) {
		vm.open_upvalues = created_upvalue;
	} else {
		prev_upvalue->next = created_upvalue;
	}

	return created_upvalue;
}

void vm_close_upvalues(const Value *last)
{
	while (vm.open_upvalues != NULL && vm.open_upvalues->location >= last) {
		UpvalueObject *upvalue = vm.open_upvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;
		vm.open_upvalues = upvalue->next;
	}
}
