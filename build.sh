#!/bin/bash

BUILD_TYPE="${BUILD_TYPE:-Debug}"
BUILD_DIR="${BUILD_DIR:-build}"

echo "Formatting code ..."
clang-format --verbose -i src/* include/*

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring $BUILD_TYPE build in $BUILD_DIR..."
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

ln -sf "$BUILD_DIR/compile_commands.json" ../compile_commands.json

echo "Building..."
make

echo "Build complete: $BUILD_DIR/gustav"
