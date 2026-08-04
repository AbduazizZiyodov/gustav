#pragma once

#include "object.h"
#include "value.h"

UpvalueObject *vm_capture_upvalue(Value *local);
void vm_close_upvalues(const Value *last);
