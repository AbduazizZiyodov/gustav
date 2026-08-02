#pragma once

#include "internal.h"

void begin_scope(void);
void end_scope(void);

void add_local(Token name);
void mark_initialized(void);

int resolve_local(Compiler *compiler, Token *name);
int resolve_upvalue(Compiler *compiler, Token *name);
