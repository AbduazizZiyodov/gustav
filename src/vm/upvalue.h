#pragma once

#include "object.h"
#include "value.h"

UpvalueObject *capture_upvalue(Value *local);
void close_upvalues(const Value *last);
