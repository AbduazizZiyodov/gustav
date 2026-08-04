#pragma once

[[gnu::format(printf, 1, 2)]] void gustav_runtime_error(const char *format, ...);
