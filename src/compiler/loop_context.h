#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint64_t loop_start;
	int exit_jump;
} LoopContext;

typedef struct {
	size_t count;
	size_t capacity;
	LoopContext *contexts;
} LoopStack;

void init_loop_stack(LoopStack *loop_stack);

void add_loop_context(LoopStack *loop_stack, LoopContext loop_scope);

void free_loop_stack(LoopStack *loop_stack);
