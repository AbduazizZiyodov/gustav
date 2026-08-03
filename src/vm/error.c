#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "internal.h"
#include "machinery.h"
#include "object.h"

__attribute__((format(printf, 1, 2))) void
runtime_error(const char *format, ...) // TODO(abduaziz): better stack trace
{
	va_list args;

	va_start(args, format);
	(void)vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	for (int i = (int)vm.frame_count - 1; i >= 0; i--) {
		CallFrame *frame = &vm.frames[i];
		FunctionObject *function = frame->closure->function;

		int line = 0;

		if (function->chunk.count > 0) {
			ptrdiff_t offset = frame->ip - function->chunk.code - 1;
			size_t instruction = offset < 0 ? 0 : (size_t)offset;

			if (instruction >= function->chunk.count) {
				instruction = function->chunk.count - 1;
			}

			line = function->chunk.lines[instruction];
		}

		(void)fprintf(stderr, "[at line %d] in ", line);

		if (function->name == NULL) {
			(void)fprintf(stderr, "script\n");
		} else {
			(void)fprintf(stderr, "%s()\n", function->name->chars);
		}
	}

	reset_stack();
}
