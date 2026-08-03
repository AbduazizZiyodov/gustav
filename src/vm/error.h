#pragma once

__attribute__((format(printf, 1, 2))) void runtime_error(const char *format,
							 ...);
