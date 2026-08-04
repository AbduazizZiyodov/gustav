#include <git.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "cli.h"
#include "common.h"
#include "log.h"
#include "version.h"
#include "vm.h"

#ifdef GUSTAV_DEBUG
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
		return vm_exit_status();
	default:
		return EXIT_SUCCESS;
	}
}

// Bob hates me
static char *read_file(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file is NULL) {
		gustav_error(EX_IOERR, "Could not open file: %s\n", path);
	}

	if (fseek(file, 0L, SEEK_END) != 0) {
		gustav_error(EX_IOERR, "Failed to seek to end: %s\n", path);
	}

	long file_size = ftell(file);

	if (file_size == -1L) {
		gustav_error(EX_IOERR, "Failed to get file size: %s\n", path);
	}

	if (fseek(file, 0L, SEEK_SET) != 0) {
		gustav_error(EX_IOERR, "Failed to rewind file: %s\n", path);
	}

	char *buffer = (char *)malloc((sizeof(char) * (size_t)file_size) + 1);
	if (buffer is NULL) {
		gustav_error(EX_IOERR, "Not enough memory to allocate buffer for %s\n", path);
	}

	size_t bytes_read = fread(buffer, sizeof(char), (size_t)file_size, file);

	if (bytes_read < (size_t)file_size) {
		if (ferror(file)) {
			(void)fclose(file);
			free(buffer);
			gustav_error(EX_IOERR, "Could not read the file %s\n", path);
		}
	}

	buffer[bytes_read] = '\0';

	if (fclose(file) != 0) {
		free(buffer);
		gustav_error(EX_IOERR, "Failed to close file: %s\n", path);
	}

	return buffer;
}

static const char *git_build_tag(void)
{
	static char tag[128];

	if (!git_IsPopulated()) {
		return "unknown";
	}

	(void)snprintf(tag, sizeof(tag), "git_branch=%s%s git_commit=%.7s", git_Branch(),
		       git_AnyUncommittedChanges() ? "-dirty" : "", git_CommitSHA1());

	return tag;
}

void show_gustav_info(void)
{
	printf("Gustav v%s [ %s | %s %s] [ %s ] | %s %s build for \"%s %s\"\n\n",
	       PROJECT_VERSION_STRING, git_build_tag(), __DATE__, __TIME__, COMPILER_VERSION,
	       OPT_LEVEL, BUILD_TYPE, OS, ARCH);

#ifdef DEBUG_STRESS_GC
	LOG_INFO("enabled  DEBUG_STRESS_GC\n\n");
#else
	LOG_INFO("disabled DEBUG_STRESS_GC\n\n");
#endif
}
