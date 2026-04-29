#!/bin/bash
set -euo pipefail
clear
readonly PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

echo "Formatting code..."
find src include -name '*.[ch]' | xargs clang-format --verbose -i

mkdir -p target

# deeebug
echo -e "\n== [build debug in build-debug] =="
cmake -B builds/build-debug --fresh \
      -DCMAKE_BUILD_TYPE=Debug \
      -DSANITIZE=ON \
      -DDEBUG_STRESS_GC=ON \
      -DDEBUG_LOG_GC=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build builds/build-debug --parallel "$(nproc)"
cp builds/build-debug/gustav target/gustav_debug
echo -e "== [/build debug in build-debug] ==\n"

# regular release build
echo -e "\n== [build release in build-release] =="
cmake -B builds/build-release --fresh \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build builds/build-release --parallel "$(nproc)"
cp builds/build-release/gustav target/gustav_release
echo -e "== [/build release in build-release] ==\n"

# valgrind with no sanitizers & no GC stress
echo -e "\n== [build valgrind in build-valgrind] =="
cmake -B builds/build-valgrind --fresh \
      -DCMAKE_BUILD_TYPE=Debug \
      -DSANITIZE=OFF \
      -DDEBUG_STRESS_GC=OFF \
      -DDEBUG_LOG_GC=OFF \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build builds/build-valgrind --parallel "$(nproc)"
cp builds/build-valgrind/gustav target/gustav_valgrind
echo -e "== [/build valgrind in build-valgrind] ==\n"

# valgrind with GC stress
echo -e "\n== [build valgrind via GC stress in build-valgrind-stress] =="
cmake -B builds/build-valgrind-stress --fresh \
      -DCMAKE_BUILD_TYPE=Debug \
      -DSANITIZE=OFF \
      -DDEBUG_STRESS_GC=ON \
      -DDEBUG_LOG_GC=OFF \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build builds/build-valgrind-stress --parallel "$(nproc)"
cp builds/build-valgrind-stress/gustav target/gustav_valgrind_stress
echo -e "== [/build valgrind via GC stress in build-valgrind-stress] ==\n"

ln -sf "builds/build-debug/compile_commands.json" compile_commands.json
