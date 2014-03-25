#!/bin/bash

set -e

# TODO: Remove this once the bug in GYP is fixed!
# https://groups.google.com/forum/#!topic/gyp-developer/m7LUzJwL9Zc
if [[ `uname` == "Darwin" ]]; then
    export CC="`which clang`"
    export CXX="`which clang++` -std=c++11 -stdlib=libc++"
fi

tools/gyp/gyp -f ninja test/test.gyp --depth . --generator-output build -D uv_library=static_library -I common.gypi
ninja -C build/out/Debug/
#ninja -C build/out/Release/
