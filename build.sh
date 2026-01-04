clang-format --verbose -i src/* include/*

rm -rf build/
mkdir build && cd build

cmake ..
make

sudo make install