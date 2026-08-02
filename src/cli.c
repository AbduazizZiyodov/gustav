#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "common.h"
#include "log.h"
#include "version.h"
#include "vm.h"

#ifdef DEBUG
#define BUILD_TYPE "DEBUG"
#else
#define BUILD_TYPE "RELEASE"
#endif

static char *read_file(const char *path);

volatile sig_atomic_t shutdown_requested = 0;

void repl(void)
{
	char line[LINE_LENGTH];
	for (;;) {
		if (shutdown_requested) {
			break; // = exit
		}

		printf("> ");
		(void)fflush(stdout);

		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break; // = exit
		}

		VM_Interpret(line);
	}
}

int run_file(const char *path)
{
	char *source = read_file(path);
	InterpretResult result = VM_Interpret(source);
	free(source);

	switch (result) {
	case INTERPRET_COMPILE_ERROR:
		return 65;
	case INTERPRET_RUNTIME_ERROR:
		return 64;
	case INTERPRET_EXIT:
		return vm.exit_status;
	default:
		return EXIT_SUCCESS;
	}
}

// Bob hates me
static char *read_file(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file is NULL) {
		gustav_error(74, "Could not open file: %s\n", path);
	}

	if (fseek(file, 0L, SEEK_END) != 0) {
		gustav_error(74, "Failed to seek to end: %s\n", path);
	}

	long file_size = ftell(file);

	if (file_size == -1L) {
		gustav_error(74, "Failed to get file size: %s\n", path);
	}

	if (fseek(file, 0L, SEEK_SET) != 0) {
		gustav_error(74, "Failed to rewind file: %s\n", path);
	}

	char *buffer = (char *)malloc((sizeof(char) * (size_t)file_size) + 1);
	if (buffer is NULL) {
		gustav_error(74,
			     "Not enough memory to allocate buffer for %s\n",
			     path);
	}

	size_t bytes_read =
		fread(buffer, sizeof(char), (size_t)file_size, file);

	if (bytes_read < (size_t)file_size) {
		if (ferror(file)) {
			(void)fclose(file);
			free(buffer);
			gustav_error(74, "Could not read the file %s\n", path);
		}
	}

	buffer[bytes_read] = '\0';

	if (fclose(file) != 0) {
		free(buffer);
		gustav_error(74, "Failed to close file: %s\n", path);
	}

	return buffer;
}

void show_gustav_info(void)
{
	printf("Gustav v%s (%s %s) [ %s ] | %s %s build for \"%s %s\"\n\n",
	       PROJECT_VERSION_STRING, __DATE__, __TIME__, COMPILER_VERSION,
	       OPT_LEVEL, BUILD_TYPE, OS, ARCH);

#ifdef DEBUG_STRESS_GC
	LOG_INFO("!!! DEBUG_STRESS_GC is enabled !!!\n\n");
#else
	LOG_INFO("!!! DEBUG_STRESS_GC is disabled !!!\n\n");
#endif
}
