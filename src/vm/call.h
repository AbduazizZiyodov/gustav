#pragma once

#include "object.h"
#include "value.h"

bool vm_call(ClosureObject *closure, int arg_count);
bool vm_call_value(Value callee, int arg_count);
bool vm_invoke_from_class(ClassObject *klass, StringObject *name, int arg_count);
bool vm_invoke(StringObject *name, int arg_count);
bool vm_bind_method(ClassObject *klass, StringObject *name);
void vm_define_method(StringObject *name);
