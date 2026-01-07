clang-format --verbose -i src/* include/*

mkdir -p build

cd build
cmake ..
ln -sf build/compile_commands.json ../compile_commands.json

make
