#!/bin/bash
set -euo pipefail

clear

readonly PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

echo "Formatting code..."
clang-format --verbose -i src/*.c include/*.h

mkdir -p target

for build_type in Debug Release; do
    build_dir="build-${build_type,,}"
    echo -e "\n== [build $build_type in $build_dir] =="

    CC=clang cmake -B "$build_dir" --fresh \
          -DCMAKE_BUILD_TYPE="$build_type" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    cmake --build "$build_dir" --parallel "$(nproc)"
    cp "$build_dir/gustav" "target/gustav_${build_type,,}"
    echo -e "== [/build $build_type in $build_dir] ==\n"
done

ln -sf "build-debug/compile_commands.json" compile_commands.json
