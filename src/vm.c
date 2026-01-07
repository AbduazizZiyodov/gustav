#include "vm.h"
#include "compiler.h"
#include "log.h"

static VM vm;

static void reset_stack(void)
{
	vm.stack_top = vm.stack;
}

void push(Value value)
{
	*vm.stack_top = value;
	vm.stack_top++;
}

Value pop(void)
{
	vm.stack_top--;
	return *vm.stack_top;
}

void init_vm(void)
{
	LOG_INFO("VM was initialized");
	reset_stack();
}

void free_vm(void)
{
	LOG_INFO("VM was freed");
}

InterpretResult interpret(const char *source)
{
	LOG_DEBUG("Compiling ...");
	compile(source);
	return INTERPRET_OK;
}
