clang-format --verbose -i src/* include/*

mkdir -p build && cd build

cmake ..
make