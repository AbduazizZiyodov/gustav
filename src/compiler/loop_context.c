#include "memory.h"
#include "loop_context.h"


void init_loop_stack(LoopStack *loop_stack)
{
	loop_stack->count = 0;
	loop_stack->capacity = 0;
	loop_stack->contexts = NULL;
}

void add_loop_context(LoopStack *loop_stack, LoopContext loop_scope)
{
	if (loop_stack->capacity < loop_stack->count + 1) {
		size_t old_capacity = loop_stack->capacity;
		loop_stack->capacity = GROW_CAPACITY(old_capacity);
		loop_stack->contexts = GROW_ARRAY(LoopContext, loop_stack->contexts, old_capacity, loop_stack->capacity);
	}

	loop_stack->contexts[loop_stack->count] = loop_scope;
	loop_stack->count++;
}

void free_loop_stack(LoopStack *loop_stack)
{
	FREE_ARRAY(LoopStack, loop_stack->contexts, loop_stack->capacity);
	init_loop_stack(loop_stack);
}
