#pragma once

#include "object.h"
#include "value.h"

bool call(ClosureObject *closure, int arg_count);
bool call_value(Value callee, int arg_count);
bool invoke_from_class(ClassObject *klass, StringObject *name, int arg_count);
bool invoke(StringObject *name, int arg_count);
bool bind_method(ClassObject *klass, StringObject *name);
void define_method(StringObject *name);
