#!/bin/bash
set -euo pipefail

readonly PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

echo "Formatting via clang-format ..."
find src include tests/unit -type f -name '*.[ch]' -print0 2>/dev/null | xargs -0 -r clang-format -i

mkdir -p target builds

# Optional Criterion unit tests (requires libcriterion-dev / pkg-config criterion)
#   ./build.sh unit-tests
#   BUILD_UNIT_TESTS=1 ./build.sh
build_unit_tests() {
  local dir="builds/build-unit-tests"
  echo -e "\n== [unit-tests -> $dir] =="
  cmake -B "$dir" --fresh -G Ninja \
        -DCMAKE_C_COMPILER=clang-19 \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_UNIT_TESTS=ON \
        -DCLANG_TIDY=OFF \
        -DSANITIZE=OFF \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build "$dir" --parallel "$(nproc)" --target test_scanner test_value
  ctest --test-dir "$dir" --output-on-failure
  echo "== [/unit-tests] =="
}

if [[ "${1:-}" == "unit-tests" ]]; then
  build_unit_tests
  exit 0
fi

readonly CONFIGS=(
#  name          build_type     clang-tidy  sanitize  GC-stress  LOG_GC
  "debug           Debug          ON          ON        OFF       OFF"
#  "debug_stress    Debug          OFF         ON        ON        ON"
  "release         Release        OFF         OFF       OFF       OFF"
#  "valgrind        Debug          OFF         OFF       OFF       OFF"
#  "valgrind_stress Debug          OFF         OFF       ON        OFF"
)

build_gustav() {
  read -r name type tidy san stress log <<<"$1"

  local dir="builds/build-${name//_/-}"

  echo -e "\n== [$name -> $dir] =="
  cmake -B "$dir" --fresh -G Ninja \
        -DCMAKE_C_COMPILER=clang-19 \
        -DCMAKE_BUILD_TYPE="$type" \
        -DCLANG_TIDY="$tidy" \
        -DSANITIZE="$san" \
        -DDEBUG_STRESS_GC="$stress" \
        -DDEBUG_LOG_GC="$log" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

  cmake --build "$dir" --parallel "$(nproc)"
  cp -f "$dir/gustav" "target/gustav_$name"
  echo "== [/$name] =="
}

for c in "${CONFIGS[@]}"; do build_gustav "$c"; done

ln -sfn builds/build-debug/compile_commands.json compile_commands.json

if [[ "${BUILD_UNIT_TESTS:-0}" == "1" ]]; then
  build_unit_tests
fi